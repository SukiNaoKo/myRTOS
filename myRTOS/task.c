#include "task.h"


readyqueue_t readqueue;
uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode)
{
    pxTopOfStack  -= 16*4 ;
    task_register_stack *Stack = (task_register_stack *) pxTopOfStack;
    Stack->PC = ((uint32_t) pxCode) &((uint32_t)0xfffffffeUL);
    Stack->xPSR = 0x01000000UL;   //xPSR的24位被置1，表示这是Thumb指令状态，实际上cm3架构只支持Thumb指令状态。
		Stack->r10 = 0xffffffffUL;
    return pxTopOfStack;
} 

__attribute__ ((__used__)) TCB_t* volatile pxCurrentTCB = NULL;


k_list_t* xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t self )
{
		
	
    TCB_t* Tcb = (TCB_t*)malloc(sizeof(TCB_t));

    Tcb->priority = uxpriority;
    Tcb->pxStack = malloc(StackSize);

    uint32_t* pxTopOfStack = Tcb->pxStack + StackSize - ((uint32_t)1);
    pxTopOfStack = ( uint32_t *) (((uint32_t)pxTopOfStack) & (~((uint32_t) 0x07)));
	
    Tcb->pxTopOfStack = pxPortInitialistStack(pxTopOfStack,pxTaskCode);
		Tcb->self_register = Tcb->pxTopOfStack;
	
    pxCurrentTCB  = Tcb;
		
		//TcbTaskTable[x]=Tcb;
		//x = x + 1;
	
    ReadyBitTable |= (1 << uxpriority);
		return readyqueue_add_task_list(Tcb);
}


void __attribute__((always_inline)) SchedulerStart(void)
{
    __asm volatile (
            " ldr r0, =0xE000ED08 	\n"/* Use the NVIC offset register to locate the stack. */
            " ldr r0, [r0] 			\n"
            " ldr r0, [r0] 			\n"
            " msr msp, r0			\n"/* Set the msp back to the start of the stack. */
            " cpsie i				\n"/* Globally enable interrupts. */
            " cpsie f				\n"
            " dsb					\n"
            " isb					\n"
            " svc 0					\n"/* System call to start first task. */
            " nop					\n"
            " .ltorg				\n"
            );
}

void __attribute__((naked)) vPortSVCHandler(void)
{
    __asm volatile (
        "   ldr r3,pxCurrentTCBConst2     \n"
        "   ldr r1,[r3]             \n"
        "   ldr r0,[r1]             \n"
        "   ldmia r0!,{r4-r11}      \n"
        "   msr psp,r0              \n"
        "   isb                     \n"
        "   mov r0,#0               \n"
        "   msr basepri,r0          \n"
        "   orr r14,#0xd            \n"
        "   bx r14                  \n"
        "                           \n"
        "   .align 4                \n"
				"pxCurrentTCBConst2: .word pxCurrentTCB				\n"
    );
}

void vTaskSwitchContext( void )
{
		x= x+1;
    pxCurrentTCB = TcbTaskTable[x%2];
}



void  __attribute__( ( naked ) )  xPortPendSVHandler( void )
{
    __asm volatile
            (
            "	mrs r0, psp							\n"
            "	isb									\n"
            "										\n"
            "	ldr	r3, pxCurrentTCBConst			\n"
            "	ldr	r2, [r3]						\n"
            "										\n"
            "	stmdb r0!, {r4-r11}					\n"
						"	str r0, [r2]						\n"            //[r2]  为TCB里面的栈帧 topofstack
            "										\n"
            "	stmdb sp!, {r3, r14}				\n"
            "	mov r0, #0							\n"
            "	msr basepri, r0						\n"
            "   dsb                                 \n"
            "   isb                                 \n"
            "	bl switch_task				\n"
            "	mov r0, #0							\n"
            "	msr basepri, r0						\n"
            "	ldmia sp!, {r3, r14}				\n"       //r3 是指向切换后的TCB  这里的sp应该是msp 内核的
            "										\n"
            "	ldr r1, [r3]						\n"
            "	ldr r0, [r1]						\n"
            "	ldmia r0!, {r4-r11}					\n"
            "	msr psp, r0							\n"                 //移除r4-r11 之后的r0地址就应该是栈帧  正好从r0开始的自动保存的栈
            "	isb									\n"
            "	bx r14								\n"
            "	nop									\n"
            "	.align 4							\n"
            "pxCurrentTCBConst: .word pxCurrentTCB	\n"
            );
}



k_list_t* add_task_list(k_list_t* list,TCB_t *new_tcb)
{
    k_list_t* new_node = (k_list_t*) malloc(sizeof (k_list_t));
    if (new_node == NULL) {
        // 内存分配失败，处理错误（例如，返回错误代码或打印错误信息）
        return NULL;
    }
    
    new_node->tcb = new_tcb;
    tos_list_add_tail(new_node, list);

    //list->prev->next = new_node;
    //new_node->prev = list->prev;


    //list->prev = new_node;
    //new_node->next = list;
    
    return  new_node;
}


 void readyqueue_init(void)
{
    uint8_t i;

    readqueue.highest_prio = 0;

    for (i = 0; i < TASK_PRIO_MAX; ++i) {
        readqueue.task_list_head[i] = (k_list_t*)malloc(sizeof(k_list_t));
        readqueue.task_list_head[i]->tcb = NULL;
        readqueue.task_list_head[i]->next = readqueue.task_list_head[i];
        readqueue.task_list_head[i]->prev = readqueue.task_list_head[i];
    }

}

 /*void list_init(k_list_t* list)
 {
     
     list->tcb = NULL;
     list->next = list;
     list->prev = list;
 }
 */


 void _list_add(k_list_t *node, k_list_t *prev, k_list_t *next)
 {
     next->prev = node;
     node->next = next;
     node->prev = prev;
     prev->next = node;
 }

 void tos_list_add(k_list_t* node, k_list_t* list)
 {
     _list_add(node, list, list->next);
 }

 void tos_list_add_tail(k_list_t* node, k_list_t* list)
 {
     _list_add(node, list->prev, list);
 }


k_list_t* readyqueue_add_task_list(TCB_t* new_tcb)
{
    
    readqueue.prio_mask |= (1<<new_tcb->priority);
	  readqueue.highest_prio = readyqueue_prio_highest_get(readqueue.prio_mask);
    return add_task_list(readqueue.task_list_head[new_tcb->priority], new_tcb);
}




void _list_del(k_list_t* prev, k_list_t* next)
{
    next->prev = prev;
    prev->next = next;
}

void _list_del_node(k_list_t* node)
{
    _list_del(node->prev, node->next);
}


int tos_list_empty(k_list_t* task_list)
{
    if (task_list->next = task_list)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}
void readyqueue_prio_remove(uint32_t task_prio)
{
    readqueue.prio_mask &= (0 << task_prio);
}


int readyqueue_prio_highest_get(uint32_t n) {
    if (n == 0) {
        // 如果n是0，则没有最高位的1，可以返回-1表示错误或特殊情况
        return -1;
    }
 
    int index = 31; // 从32位数的最高位开始
 
    // 通过位运算逐步检查每一位，直到找到最高位的1
    while ((n & (1 << index)) == 0) {
        index--;
    }
 
    return index;
}

void readyqueue_remove(k_list_t* task)
{
    k_prio_t task_prio;

    task_prio = task->tcb->priority;

    _list_del_node(task);     //删除节点

    if (tos_list_empty(task)) {
        readyqueue_prio_remove(task_prio);
    }

    if (task_prio == readqueue.highest_prio) {
        readqueue.highest_prio = readyqueue_prio_highest_get(readqueue.prio_mask);
    }
}


void switch_task(void)
{
    if (readqueue.task_list_head[readqueue.highest_prio]->tcb == NULL)
    {
        readqueue.task_list_head[readqueue.highest_prio] = readqueue.task_list_head[readqueue.highest_prio]->next;
    }
    pxCurrentTCB = readqueue.task_list_head[readqueue.highest_prio]->tcb;

    readqueue.task_list_head[readqueue.highest_prio] = readqueue.task_list_head[readqueue.highest_prio]->next;
		
		
		
}


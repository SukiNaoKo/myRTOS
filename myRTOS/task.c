#include "task.h"



uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode)
{
    pxTopOfStack  -= 16 ;
    task_register_stack *Stack = (task_register_stack *) pxTopOfStack;
    Stack->PC = ((uint32_t) pxCode) &((uint32_t)0xfffffffeUL);
    Stack->xPSR = 0x01000000UL;   //xPSR的24位被置1，表示这是Thumb指令状态，实际上cm3架构只支持Thumb指令状态。
		Stack->r10 = 0xffffffffUL;
    return pxTopOfStack;
} 

__attribute__ ((__used__)) TCB_t* volatile pxCurrentTCB = NULL;

xheap theheap = {
    .tail = NULL,
    .allsize = config_heap
};

void xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t self )
{
		
	
    TCB_t* Tcb = (TCB_t*)malloc(sizeof(TCB_t));

    Tcb->priority = uxpriority;
    Tcb->pxStack = malloc(StackSize);

    uint32_t* pxTopOfStack = Tcb->pxStack + StackSize - ((uint32_t)1);
    pxTopOfStack = ( uint32_t *) (((uint32_t)pxTopOfStack) & (~((uint32_t) 0x07)));
	
    Tcb->pxTopOfStack = pxPortInitialistStack(pxTopOfStack,pxTaskCode);
		Tcb->self_register = Tcb->pxTopOfStack;
    pxCurrentTCB  = Tcb;
		
		TcbTaskTable[x]=Tcb;
		x = x + 1;
	
    ReadyBitTable |= (1 << uxpriority);

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
            "	str r0, [r2]						\n"
            "										\n"
            "	stmdb sp!, {r3, r14}				\n"
            "	mov r0, #0							\n"
            "	msr basepri, r0						\n"
            "   dsb                                 \n"
            "   isb                                 \n"
            "	bl vTaskSwitchContext				\n"
            "	mov r0, #0							\n"
            "	msr basepri, r0						\n"
            "	ldmia sp!, {r3, r14}				\n"
            "										\n"
            "	ldr r1, [r3]						\n"
            "	ldr r0, [r1]						\n"
            "	ldmia r0!, {r4-r11}					\n"
            "	msr psp, r0							\n"
            "	isb									\n"
            "	bx r14								\n"
            "	nop									\n"
            "	.align 4							\n"
            "pxCurrentTCBConst: .word pxCurrentTCB	\n"
            );
}


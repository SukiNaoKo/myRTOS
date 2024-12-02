//#include "core_cm3.h"
#include<stdint.h>
#include<stdlib.h>


#define Class(name)   \
typedef struct name name;\
struct name
#define vPortSVCHandler SVC_Handler
#define config_heap 8*1024

static uint8_t allheap[config_heap];
static TCB_t* pxCurrentTCB; 

Class(TCB_t)
{
    volatile uint32_t *pxTopOfStack;
    task_register_stack  *self_register;
    uint32_t priority;
    uint32_t *pxStack;
};


typedef TCB_t   *TaskHandle_t;
__MINGW_ATTRIB_USED TCB_t* volatile pxCurrentTCB = NULL;

typedef void(*TaskFunction_t)(void *);



Class(heap_node)
{
    heap_node *next;
    size_t blocksize;
};

Class(xheap)
{
    heap_node head;
    heap_node *tail;
    size_t allsize;
};

xheap theheap = {
    .tail = NULL,
    .allsize = config_heap
};





Class(task_register_stack)
{
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    //automatic stacking
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t LR;
    uint32_t PC;
    uint32_t xPSR;
};


uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode)
{
    pxTopOfStack  = pxTopOfStack -16;
    task_register_stack * Stack = (task_register_stack *) pxTopOfStack;
    Stack->PC = ((uint32_t) pxCode) &((uint32_t)0xfffffffeUL);
    Stack->xPSR = 0x01000000UL;   //xPSR的24位被置1，表示这是Thumb指令状态，实际上cm3架构只支持Thumb指令状态。
    return pxTopOfStack;
} 


void xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t)
{
    TCB_t* Tcb = (TCB_t*)malloc(sizeof(TCB_t));

    Tcb->priority = uxpriority;
    Tcb->pxStack = malloc(sizeof(StackSize));

    uint32_t* pxTopOfStack = Tcb->pxStack + StackSize - ((uint32_t)1);
    
    pxTopOfStack = pxPortInitialistStack(pxTopOfStack,pxTaskCode);

    pxCurrentTCB  = Tcb;
}



void __attribute__((always_inline)) SchedulerStart(void)
{
    __asm volatile(
        "   ldr r0,0xE000ED08   \n"
        "   ldr r0,[r0]         \n"
        "   ldr r0,[r0]         \n"
        "   msr msp,r0          \n"
        "   cpsie i             \n"
        "   cpsie f             \n"
        "   dsb                 \n"
        "   isb                 \n"
        "   svc 0               \n"   //触发svc中断
        " nop                   \n"
        " .ltorg                \n"
    );
}

void __attribute__((naked)) vPortSVCHandler(void)
{
    __asm volatile (
        "   ldr r3,pxCurrentTCB     \n"
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
    );
}



int main()
{



    return 0;
}



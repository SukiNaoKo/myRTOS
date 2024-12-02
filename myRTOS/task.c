#include "task.h"

uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode)
{
    pxTopOfStack  = pxTopOfStack -16;
    task_register_stack * Stack = (task_register_stack *) pxTopOfStack;
    Stack->PC = ((uint32_t) pxCode) &((uint32_t)0xfffffffeUL);
    Stack->xPSR = 0x01000000UL;   //xPSR的24位被置1，表示这是Thumb指令状态，实际上cm3架构只支持Thumb指令状态。
    return pxTopOfStack;
} 


void xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t self )
{
    TCB_t* Tcb = (TCB_t*)malloc(sizeof(TCB_t));

    Tcb->priority = uxpriority;
    Tcb->pxStack = malloc(sizeof(StackSize));

    uint32_t* pxTopOfStack = Tcb->pxStack + StackSize - ((uint32_t)1);
    
    pxTopOfStack = pxPortInitialistStack(pxTopOfStack,pxTaskCode);

    pxCurrentTCB  = Tcb;

    ReadyBitTable |= (1 << uxpriority);

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

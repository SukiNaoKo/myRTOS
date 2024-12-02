
#include"global.h"

#define config_heap 8*1024
#define vPortSVCHandler SVC_Handler
static uint8_t allheap[config_heap];
typedef TCB_t   *TaskHandle_t;
__MINGW_ATTRIB_USED TCB_t* volatile pxCurrentTCB = NULL;
static TCB_t* pxCurrentTCB; 
static uint32_t  ReadyBitTable;

typedef void(*TaskFunction_t)(void *);

Class(TCB_t)
{
    volatile uint32_t *pxTopOfStack;
    task_register_stack  *self_register;
    uint32_t priority;
    uint32_t *pxStack;
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


uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode);

void xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t self);
void __attribute__((always_inline)) SchedulerStart(void);
void __attribute__((naked)) vPortSVCHandler(void);
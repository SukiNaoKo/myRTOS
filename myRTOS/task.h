
#include "global.h"

#define config_heap 8*1024

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define TASK_PRIO_MAX   10u
static uint8_t allheap[config_heap];

static int x = 0;

static uint32_t  ReadyBitTable;

typedef void (*TaskFunction_t)(void *);

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



Class(TCB_t)
{
    volatile uint32_t *pxTopOfStack;
    uint32_t priority;
    uint32_t *pxStack;
		task_register_stack  *self_register;
};









typedef TCB_t   *TaskHandle_t;


static TCB_t* TcbTaskTable[2];


extern TCB_t* volatile pxCurrentTCB;

typedef uint8_t             k_prio_t;

typedef struct k_list_node_st
{
    struct k_list_node_st *next;
    struct k_list_node_st *prev;
    TCB_t *tcb;
} k_list_t;


typedef struct readyqueue_st
{
    k_list_t*    task_list_head[TASK_PRIO_MAX];     //优先级列表
    uint32_t    prio_mask;           //用于指示系统目前所使用优先级的优先级表
    k_prio_t    highest_prio;        //最高优先级
} readyqueue_t;





uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode);

void __attribute__((always_inline)) SchedulerStart(void);
void __attribute__((naked)) vPortSVCHandler(void);
void __attribute__((naked)) xPortPendSVHandler( void);
void vTaskSwitchContext( void );


void readyqueue_init(void);
void list_init(k_list_t* list);
k_list_t* add_task_list(k_list_t* list,TCB_t *new_tcb);
k_list_t* readyqueue_add_task_list(TCB_t* new_tcb);
k_list_t* xTaskCreate(TaskFunction_t pxTaskCode,uint16_t StackSize, uint32_t uxpriority, TaskHandle_t self);
uint32_t * pxPortInitialistStack(uint32_t* pxTopOfStack,TaskFunction_t pxCode);
void _list_add(k_list_t* node, k_list_t* prev, k_list_t* next);
void tos_list_add(k_list_t* node, k_list_t* list);
void tos_list_add_tail(k_list_t* node, k_list_t* list);
void _list_del(k_list_t* prev, k_list_t* next);
void _list_del_node(k_list_t* node);
int tos_list_empty(k_list_t* task_list);
void readyqueue_prio_remove(uint32_t task_prio);
int readyqueue_prio_highest_get(uint32_t n);
void readyqueue_remove(k_list_t* task);
void switch_task(void);

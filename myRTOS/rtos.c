
#include "task.h"
#include <stdio.h>


void task1(void)
{
    printf("task1----running\n\r");
}



void main(void)
{
    TaskHandle_t task_handle = NULL;
    xTaskCreate(task1,1000,1,task_handle);


    


}
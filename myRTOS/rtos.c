
#include "main.h"
#include "gpio.h"
#include "task.h"
#include "stdio.h"
#include "core_cm3.h"  // CMSIS库头文件，包含内核寄存器定义

#define  NVIC_INT_CTRL      0xE000Ed04 //PendSV中断控制器地址
#define  NVIC_PENDSV_SET    0x10000000 //PendSV触发的值
#define  NVIC_SYSPRI2       0xE000Ed22 //PendSV优先级控制地址
#define  NVIC_PENDSV_PRI    0x000000ff //PendSV设置为最低优先值
#define  MEM32(addr)        *(volatile unsigned long *)(addr)
#define  MEM8(addr)         *(volatile unsigned char *)(addr)


void switchTask(void)
{
		MEM32(NVIC_INT_CTRL) = NVIC_PENDSV_SET;  //触发PendSV	   
}
static int s,k ;

void SystemClock_Config(void);

void task1(void)
{
	
	while(1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);

		HAL_Delay(1000);
		switchTask();
		HAL_Delay(500);
		
		k++;
		
	}
		
}
void task2(void) 
{
	
	while(1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	
		HAL_Delay(2000);
		s++;
		switchTask();
	}
}

TaskHandle_t task_handle = NULL;

int main(void)
{
	s= 0;
	k = 0;
  HAL_Init();
	SystemClock_Config();
  MX_GPIO_Init();
	MEM8(NVIC_SYSPRI2) =  NVIC_PENDSV_PRI;//设置PendSV优先级
	xTaskCreate((TaskFunction_t)task2,100,1,task_handle);
	xTaskCreate((TaskFunction_t)task1,100,1,task_handle);
	//switchTask();
	SchedulerStart();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


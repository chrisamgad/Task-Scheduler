#include  <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
//#include "structsdeclarations.h"

#include "stm32l4xx_it.h"
uint8_t tt;
uint8_t newline= '\n';
uint8_t carriage= '\r';
uint8_t IntegerDigits[2];
uint8_t DecimalDigits[2];
uint8_t out[] = {0,0,'.',0,0};

int ENTERPRESSED=0;

uint8_t digit1;
uint8_t digit2;
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

bool ExceedThreshold=false;
UART_HandleTypeDef huart1;

typedef void (*fptr)();

struct taskQueue {
	int CURRENTSIZE;
	int MAXSIZE;
	struct task *tasks;
};

struct taskQueue ready_queue;
struct taskQueue delay_queue;

struct task {
    int priority;
    fptr pointer_to_func;
    int delay;
};
//extern char c;
int ticks=0;
bool Reached1sec=false;
bool flag=true;
struct task tasktoberemoved;

void  insertintodelay(fptr f, int delay, int prio){
    delay_queue.tasks[delay_queue.CURRENTSIZE].delay=delay+1;
    delay_queue.tasks[delay_queue.CURRENTSIZE].priority= prio;
    delay_queue.tasks[delay_queue.CURRENTSIZE].pointer_to_func =f;
    delay_queue.CURRENTSIZE++;
    
    for (int i=0; i<delay_queue.CURRENTSIZE;i++){
        for (int j=0; j<delay_queue.CURRENTSIZE;j++){
            if (delay_queue.tasks[i].delay < delay_queue.tasks[j].delay && i!=j){

                struct task temp = delay_queue.tasks[i];
	            delay_queue.tasks[i] = delay_queue.tasks[j];
	            delay_queue.tasks[j] = temp;
            }
        }
    }
}

void QueTask(fptr f, int Priority)
{
    ready_queue.tasks[ready_queue.CURRENTSIZE].priority=Priority;
    ready_queue.tasks[ready_queue.CURRENTSIZE].pointer_to_func =f;
    ready_queue.CURRENTSIZE++;

    for (int i=0; i<ready_queue.CURRENTSIZE;i++){
        for (int j=0; j<ready_queue.CURRENTSIZE;j++){
            if (ready_queue.tasks[i].priority > ready_queue.tasks[j].priority && i!=j){

                struct task temp = ready_queue.tasks[i];
	            ready_queue.tasks[i] = ready_queue.tasks[j];
	            ready_queue.tasks[j] = temp;
            }
        }
    }

}

void ReRunMe(int delay){
    if (delay==0) {
        QueTask(tasktoberemoved.pointer_to_func, tasktoberemoved.priority);
    }
    else {
        insertintodelay(tasktoberemoved.pointer_to_func, delay, tasktoberemoved.priority);
    }
}

void Toggle_LED(){
	
		if(ExceedThreshold==true)
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
		else
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
		flag=false;
		ReRunMe(1);

}
int Temp_Threshold=0;

void Compare_Threshold(){

	Temp_Threshold = ((digit1-'0')*10) + (digit2-'0');
	int TempFromSensor = ((out[0]-'0')*10) + (out[1]-'0');
	if (TempFromSensor >= Temp_Threshold)
		ExceedThreshold=true;
	else
		ExceedThreshold=false;
	
	flag=false;
	ReRunMe(600);


}

void Read_Temperature()
{

		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, IntegerDigits, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, IntegerDigits+1, 1, 10);
		
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, DecimalDigits, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, DecimalDigits+1, 1, 10);
		
		out[1] = IntegerDigits[1] %10 +'0';
		IntegerDigits[1] = IntegerDigits[1]/10;
		out[0] = IntegerDigits[1] %10 +'0';
		IntegerDigits[1] = IntegerDigits[1]/10;
		
		uint8_t ShiftedBits= DecimalDigits[1] >>6;
		if(ShiftedBits==0x0)
		{
			out[4]=0 +'0';
			out[3]=0 +'0';
		}
		else if (ShiftedBits==0x1)
		{
			out[4]=5 +'0';
			out[3]=2 +'0';
		}
		else if (ShiftedBits==0x2)
		{
			out[4]=0 +'0';
			out[3]=5 +'0';
		}
		else if (ShiftedBits==0x3)
		{
			out[4]=5 +'0';
			out[3]=7 +'0';
		}
		HAL_UART_Transmit(&huart1,out, sizeof(out), 100);
		HAL_UART_Transmit(&huart1,&newline,sizeof(newline),200);
		HAL_UART_Transmit(&huart1,&carriage,sizeof(carriage),200);
		flag=false;
		ReRunMe(600);
	

}

void Nothing(){
    
if(flag==true)
		{
			flag=false;
		}
		
}
 void init()
{
    
		int size=10;
    ready_queue.CURRENTSIZE=0;
    ready_queue.MAXSIZE=size;
    ready_queue.tasks = (struct task*)malloc(size * sizeof(struct task));

    delay_queue.CURRENTSIZE=0;
    delay_queue.MAXSIZE=size;
    delay_queue.tasks = (struct task*)malloc(size * sizeof(struct task));

    for(int i=0;i< size; i++)
        {
            ready_queue.tasks[i].priority=-1;
            ready_queue.tasks[i].delay=99999;
            ready_queue.tasks[i].pointer_to_func = &Nothing;

            delay_queue.tasks[i].priority=-1;
            delay_queue.tasks[i].delay=99999;
            delay_queue.tasks[i].pointer_to_func = &Nothing;
        }  
} 

void dispatch(){
    
	if(flag==true && ENTERPRESSED==1)
	{	
		if (ready_queue.CURRENTSIZE > 0){
					 
				tasktoberemoved = ready_queue.tasks[0];
					for (int i=0;i<ready_queue.CURRENTSIZE;i++)
							ready_queue.tasks[i] = ready_queue.tasks[i+1];
			
					ready_queue.CURRENTSIZE--;
					ready_queue.tasks[ready_queue.CURRENTSIZE].priority = -1;
					ready_queue.tasks[ready_queue.CURRENTSIZE].delay=99999;
					ready_queue.tasks[ready_queue.CURRENTSIZE].pointer_to_func = &Nothing;
					
					tasktoberemoved.pointer_to_func(); 
				 
			}
		 else if (ready_queue.CURRENTSIZE==0)//if no tasks in ready queue in this tick now
					{
							Nothing();			
					}
				int i=0;
				while(delay_queue.CURRENTSIZE!=0)
							{
								if(delay_queue.tasks[i].delay== 99999)
											break;
									if(delay_queue.tasks[0].delay==0)
											{
													QueTask(delay_queue.tasks[0].pointer_to_func,delay_queue.tasks[0].priority);
													
													for (int i=0;i<delay_queue.CURRENTSIZE;i++)
															delay_queue.tasks[i] = delay_queue.tasks[i+1];                       
													delay_queue.CURRENTSIZE--;
													delay_queue.tasks[delay_queue.CURRENTSIZE].priority = -1;
													delay_queue.tasks[delay_queue.CURRENTSIZE].delay=99999;
													delay_queue.tasks[delay_queue.CURRENTSIZE].pointer_to_func = &Nothing;
													i--;
											} 
									i++;                    
							}
	}
}


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
	

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  IntegerDigits[0]=0x11;
	DecimalDigits[0]=0x12;
	
	init();

	QueTask(&Read_Temperature,8);
	QueTask(&Compare_Threshold,7);
	QueTask(&Toggle_LED,6);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
  while (1)
  {
		dispatch();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /**Enable MSI Auto calibration 
  */
	
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()*0.05); //0.05 represets 1 tick (50ms)
/**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

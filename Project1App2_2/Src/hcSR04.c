#include  "hcSR04.h"
#include "main.h"
#include "stdbool.h"

extern bool flag;
uint8_t FirstCaptured=0;
uint32_t IC_Val1=0;
uint32_t IC_Val2=0;
uint32_t Difference=0;
uint8_t Distance=0;
bool FarAwayFromSensor=false;
int delayTicks=0;
extern TIM_HandleTypeDef htim1;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // if the interrupt source is channel1
	{	
		CaptureInput();
	}
}

//the delay is being used to generate 10us that is needed in the beginning of the Trig Pin acccording to the datasheet
void delay(uint16_t time){
__HAL_TIM_SetCounter(&htim1,0);
	while(__HAL_TIM_GetCounter (&htim1)<time);

}

void WaitForEachTick (void){

	if(FarAwayFromSensor==false)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
	else if(FarAwayFromSensor==true)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);
	//delay(5000);// 50ms
	
	flag=false;
	ReRunMe(0); 
	
}

void ToggleAlarm (void){

	
	if(FarAwayFromSensor==false)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);
	else if(FarAwayFromSensor==true)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);
	
	flag=false;
	ReRunMe(delayTicks); //Changes each time the new Distance is read from sensor
	
}
//***********************ReadDistance******************//
void ReadDistance (void){

	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	delay(10);  // Wait for 10 us
	
	FarAwayFromSensor=false;

	if (Distance==1)
		delayTicks=5;
	else if(Distance>=2 && Distance<4)
		delayTicks=10;
	else if(Distance>=4 && Distance<6)
		delayTicks=20;
	else if(Distance>=6 && Distance<8)
		delayTicks=30;
	else if(Distance>=8 && Distance<10)
		delayTicks=40;
	else if(Distance>=10)
		FarAwayFromSensor=true;


	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);  
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);
	flag=false;
	ReRunMe(20); //reads every 1 second
}




void CaptureInput(void)
	{
		if (FirstCaptured==0) // if the first value is not captured
		{
			IC_Val1 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1); //(first we catch RISING edge) read the first value
			FirstCaptured = 1;  // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
		}
		else if (FirstCaptured==1)   // 
		{
			IC_Val2 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);  // read second value
			__HAL_TIM_SET_COUNTER(&htim1, 0);  // reset the counter

			if (IC_Val2 > IC_Val1)
				Difference = IC_Val2-IC_Val1;
			
			else if (IC_Val1 > IC_Val2)
				Difference = (0xffff - IC_Val1) + IC_Val2;
			
			Distance = Difference * 0.034/2;
			
			FirstCaptured = 0; 
			__HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim1, TIM_IT_CC1);
		}
		
		
}
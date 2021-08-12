#ifndef __hcSR04
#define __hcSR04

#include "stm32l4xx_hal.h"


#define    TRIG_PIN             GPIO_PIN_9
#define     TRIG_PORT           GPIOA



void delay(uint16_t time);
void ReadDistance (void);
void ToggleAlarm (void);
void WaitForEachTick (void);
void CaptureInput(void);


#endif

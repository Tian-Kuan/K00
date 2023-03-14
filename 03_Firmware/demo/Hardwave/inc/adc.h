#ifndef _ADC_H
#define _ADC_H
#include "gd32f4xx.h"
 
void ADC_Init(void);//adc³õÊ¼»¯º¯Êý
void ADC_Test(void);
void ADC_OffsetCal(void);

void ADC_Sample(uint16_t period, uint16_t num, int16_t *pAvgI, int16_t *pAvgQ);

#endif


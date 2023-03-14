/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               adc.c
** Latest modified Date:    2022-5-26
** Latest Version:          1.0
** Descriptions:            Used for Inner Analog Sample
**                          ����У׼ʱ������10Hz����Ƶ�ʣ���������ʱ����1Hz����Ƶ�ʽ��м��ӡ�
**							����Ƶ�ʽϵͣ�ʹ���жϷ�ʽ��������
**							ʹ��Timer2�������ڴ����ɼ���ADC�����жϱ������ݣ��ڳ����д�������
**
**--------------------------------------------------------------------------------------------------------
** Created by:              Wuyong
** Created date:            2022-5-26
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                     
** Descriptions:            
**
*********************************************************************************************************/

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"
#include "systick.h"

#include "adc.h"
#include "console.h"
#include "timer.h"
#include "pwm.h"


/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
typedef struct {
	uint16_t	dataI;	//Sin
	uint16_t	dataQ;	//Cos
	uint16_t	dataC;	//Current
	uint16_t    offsetI;	//Sinƫ��
	uint16_t	offsetQ;	//Cosƫ��
	uint16_t    offsetC;	//����ƫ��
	uint16_t	count;	//��ȡ����
	uint16_t    rsvd;	//����
}tADCDatas;


/** ---	Start of Para define 	---------------------------------------------------------------------  **/
tADCDatas g_stADCData;

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	adc_init
*	�������ܣ�	��ʼ��ADC�ɼ�
*	��ڲ�����	None
*	���ز�����	None
*	˵����      ��ʹ��ADC012������DMA����
*               PC0 - ADC_DQ    IN10
*               PC1 - ADC_DI    IN11
*               PC2 - IV_TEST   IN12
************************************************************/
void ADC_Init(void)
{
	int i;
	
    rcu_periph_clock_enable(RCU_GPIOC);//ʹ��GPIOC��ʱ��

    rcu_periph_clock_enable(RCU_ADC0);//ʹ��ADC0��ʱ�� 
    rcu_periph_clock_enable(RCU_ADC1);//ʹ��ADC1��ʱ��
	rcu_periph_clock_enable(RCU_ADC2);//ʹ��ADC2��ʱ��	

    adc_clock_config(ADC_ADCCK_HCLK_DIV20);//����ADC��ʱ��: 168MHz/20 = 8.4MHz, ADC���ʱ��40MHz
	 
	gpio_mode_set(GPIOC,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);//ģ������
 
    adc_channel_length_config(ADC0,ADC_REGULAR_CHANNEL,1);//ͨ����������
    adc_channel_length_config(ADC1,ADC_REGULAR_CHANNEL,1);
	adc_channel_length_config(ADC2,ADC_REGULAR_CHANNEL,1);		
    
	adc_special_function_config(ADC0,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);//�ر�ADCɨ��ģʽ���ܺ������ɼ����ܣ�SM=0, CTN=0
	adc_special_function_config(ADC1,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);
	adc_special_function_config(ADC2,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);
	
	adc_discontinuous_mode_config(ADC0,ADC_CHANNEL_DISCON_DISABLE,1);//����ADC�ļ�ϲɼ�ģʽ��DISRC=0

	adc_regular_channel_config(ADC0,0,ADC_CHANNEL_11,ADC_SAMPLETIME_480);//ADC_DI: ����ʱ�䣺480/8.4MHz = 57.1us��������Ƶ��: 8.4MHz/
    adc_regular_channel_config(ADC1,0,ADC_CHANNEL_10,ADC_SAMPLETIME_480);//ADC_DQ
	adc_regular_channel_config(ADC2,0,ADC_CHANNEL_12,ADC_SAMPLETIME_480);//IV_TEST

    adc_external_trigger_config(ADC0,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_RISING);	//ʹ��ADC0���ⲿTimer����������ADC���ô���
	adc_external_trigger_config(ADC1,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_DISABLE);	
	adc_external_trigger_config(ADC2,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_DISABLE);
	
    adc_external_trigger_source_config(ADC0,ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_T4_CH0);//ADCʹ����Timer4 CH0���д���
    
    adc_data_alignment_config(ADC0,ADC_DATAALIGN_RIGHT);//ADC����У׼����
    adc_data_alignment_config(ADC1,ADC_DATAALIGN_RIGHT);
	adc_data_alignment_config(ADC2,ADC_DATAALIGN_RIGHT);
 
    adc_sync_mode_config(ADC_ALL_REGULAL_PARALLEL);//����ADCͬ��ģʽ
	
	adc_oversample_mode_config(ADC0,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	//����ADCΪ256�������������16bit
	adc_oversample_mode_config(ADC1,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	
	adc_oversample_mode_config(ADC2,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	
	
	adc_oversample_mode_enable(ADC0);	//ʹ��ADC������ģʽ
	adc_oversample_mode_enable(ADC1);
	adc_oversample_mode_enable(ADC2);

    adc_enable(ADC0);//ʹ��ADC0�ӿ�
	adc_enable(ADC1);
	adc_enable(ADC2);	

	for(i=0;i<280;i++){};//�ȴ�����14��ADCCLK�Ա�֤ADC�ȶ�: 168MHz/8.4MHz*14= 280

    adc_calibration_enable(ADC0);//ADC0��У׼��λ������У׼
    adc_calibration_enable(ADC1); 
	adc_calibration_enable(ADC2);		
		
	Timer4_Init();
		
	g_stADCData.offsetI = 0x8000;
	g_stADCData.offsetQ = 0x8000;
	g_stADCData.offsetC = 0;
		
	nvic_irq_enable(ADC_IRQn, 6, 6);
}

/************************************************************
*	�������ƣ�	ADC_start
*	�������ܣ�	����ADC�ɼ�
*	��ڲ�����	period - ���ݲɼ������ڣ�ms��
*	���ز�����	None
*	˵����      
************************************************************/
void ADC_start(uint16_t period)
{
	g_stADCData.count = 0;
	adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);//���ADC�жϱ�־
	adc_interrupt_enable(ADC0, ADC_INT_EOC);//ʹ��ADC0�ж�
	
	Timer4_SetPeriod(period);
	timer_enable(TIMER4);//ʹ��Timer4
}

/************************************************************
*	�������ƣ�	ADC_stop
*	�������ܣ�	ֹͣADC�ɼ�
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void ADC_stop(void)
{
	timer_disable(TIMER4);//����Timer4
	adc_interrupt_disable(ADC0, ADC_INT_EOC);	//����ADC0�ж�
}

/************************************************************
*	�������ƣ�	ADC_Test
*	�������ܣ�	ADC�ɼ�����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      ADC����ʱ�䣺480clk��ת��ʱ��12clk��ƽ��������256��
*				ADCʱ��Ƶ�ʣ�8.4MHz�����ADCһ��ת����Ҫ��(480+12)*256 = 125952clk��Լ15ms��
************************************************************/
void ADC_Test(void)
{
	uint16_t period = 500;	//�������ڣ�����
	int iData;
	uint16_t count=0, num=0;	//��������
	float fData;
	char ch;
	uint32_t sumI=0, sumQ=0, sumC=0;
	uint16_t dataI, dataQ, dataC;
	uint16_t maxI=0, maxQ=0, maxC=0;
	uint16_t minI=0xFFFF, minQ=0xFFFF, minC=0xFFFF;
		
	cprintf("\nStart Inner ADC Test\n");
	cprintf("Enter the Sample period(Range:100~10000ms)(Default:%dms): ",period);
	if(ERROR != CS_GetIntNum(&iData))
	{
		if(iData>=100 && iData<=10000)
			period = (uint16_t) iData;
	}
	cprintf("  Period Set: %dms\n", period);
	cprintf("Sample Started, Press Any key to Exit...\n");
	cprintf("  Count\t ADC_DCI(mV)\t ADC_DCQ(mV)\t ADC_CUR(mA)\n");
	CS_ClearChar();
	
	ADC_start(period);	//��������
	while(ERROR == CS_QueryChar(&ch))
	{
		if(count != g_stADCData.count)	//�����仯����ʾ����
		{
			count = g_stADCData.count;
			dataI = g_stADCData.dataI;
			dataQ = g_stADCData.dataQ;
			dataC = g_stADCData.dataC;
			
			cprintf("  %d:\t", count);			

			fData = (int16_t)(dataI-g_stADCData.offsetI)*(3300/65535.0);			
			cprintf(" 0x%04X(%.2f)\t", dataI, fData);			

			fData = (int16_t)(dataQ-g_stADCData.offsetQ)*(3300/65535.0);	
			cprintf(" 0x%04X(%.2f)\t", dataQ, fData);
			
			//INA381A1�Ŵ�20V/V��������0.47��
			fData = (int16_t)(dataC-g_stADCData.offsetC)*(3300.0/65535.0/20.0/0.47);	//mA
			cprintf(" 0x%04X(%.2f)\n", dataC, fData);
			
			if(num<0xFFFF)
			{
				num ++;
				sumI += dataI;
				sumQ += dataQ;
				sumC += dataC;
			}
			if(minI > dataI)
				minI = dataI;
			if(maxI < dataI)
				maxI = dataI;
			if(minQ > dataQ)
				minQ = dataQ;
			if(maxQ < dataQ)
				maxQ = dataQ;
			if(minC > dataC)
				minC = dataC;
			if(maxC < dataC)
				maxC = dataC;
		}		
	}
	ADC_stop();
	
	cprintf("  PHASE I(sin) - min: 0x%04X; max: 0x%04X; avg: 0x%04X\n", minI, maxI, (sumI/num));
	cprintf("  PHASE Q(cos) - min: 0x%04X; max: 0x%04X; avg: 0x%04X\n", minQ, maxQ, (sumQ/num));
	cprintf("  PHASE C(cur) - min: 0x%04X; max: 0x%04X; avg: 0x%04X\n", minC, maxC, (sumC/num));
	cprintf("Sample Stopped\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	ADC_Sample
*	�������ܣ�	����period���ڣ�����num�����ݽ��з���
*	��ڲ�����	period  - �������ڣ���λms
*				num     - ��������
*				pDataI	- ���ص�I�ź�ƽ��ֵ
*				pDataQ  - ���ص�Q�ź�ƽ��ֵ
*	���ز�����	None
*	˵����		
************************************************************/
void ADC_Sample(uint16_t period, uint16_t num, int16_t *pAvgI, int16_t *pAvgQ)
{
    uint32_t sumI=0, sumQ=0;
	int i;
    uint16_t count = 0;		//��������
	
	i  = num;
 	ADC_start(period);	//��������,����500ms��������
	while(i)
	{
		if(count != g_stADCData.count)	//�����仯����ʾ����
		{
			count = g_stADCData.count;			
			sumI += g_stADCData.dataI;
			sumQ += g_stADCData.dataQ;
            i--;	
		}		
	}
	ADC_stop(); 	
	
	*pAvgI =(int16_t) ((uint16_t)(sumI/num)-g_stADCData.offsetI);
	*pAvgQ =(int16_t) ((uint16_t)(sumQ/num)-g_stADCData.offsetQ);
}

/************************************************************
*	�������ƣ�	ADC_SampleAll
*	�������ܣ�	����period���ڣ�����num�����ݣ���ͨ�������з�����
*	��ڲ�����	period  - �������ڣ���λms
*				num     - ��������
*				pDataI	- ���ص�I�ź�ƽ��ֵ
*				pDataQ  - ���ص�Q�ź�ƽ��ֵ
*	���ز�����	None
*	˵����		
************************************************************/
void ADC_SampleAll(uint16_t period, uint16_t num, int16_t *pAvgI, int16_t *pAvgQ, int16_t *pAvgC)
{
    uint32_t sumI=0, sumQ=0, sumC=0;
	int i;
    uint16_t count = 0;		//��������	
	
	i  = num;
 	ADC_start(period);	//��������,����500ms��������
	while(i)
	{
		if(count != g_stADCData.count)	//�����仯����ʾ����
		{
			count = g_stADCData.count;			
			sumI += g_stADCData.dataI;
			sumQ += g_stADCData.dataQ;
			sumC += g_stADCData.dataC;
			i--;
		}		
	}
	ADC_stop(); 
	
	*pAvgI =(int16_t)((uint16_t) (sumI/num) - g_stADCData.offsetI);
	*pAvgQ =(int16_t)((uint16_t) (sumQ/num) - g_stADCData.offsetQ);
	*pAvgC =(int16_t)((uint16_t) (sumC/num) - g_stADCData.offsetC);
}

/************************************************************
*	�������ƣ�	ADC_OffsetCal
*	�������ܣ�	��ADC��ƫ�ƽ��б궨
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void ADC_OffsetCal(void)
{
	uint16_t period=500;
	int16_t avgI, avgQ, avgC;
	int iData;
	
    PWM_Enable(0);  //����ͨ����ʹ��
	
	cprintf("\nStart Inner ADC Offset Calibrate\n");
	cprintf("Enter the Sample period(Range:100~10000ms)(Default:%dms): ",period);
	
	if(ERROR != CS_GetIntNum(&iData))
	{
		if(iData>=100 && iData<=10000)
			period = (uint16_t) iData;
	}
	cprintf("  Period Set: %dms\n", period);
	g_stADCData.offsetI = 0;
	g_stADCData.offsetQ = 0;
	g_stADCData.offsetC = 0;
	
	cprintf("Start Calibrating, Wait about %dsec...\n", ((uint32_t)period*16)/1000+2);
    delay_1ms(2000);    //�ȴ������ȶ���2��  
	ADC_SampleAll(period, 16, &avgI, &avgQ, &avgC);
	
	g_stADCData.offsetI = (uint16_t)avgI;
	g_stADCData.offsetQ = (uint16_t)avgQ;
	g_stADCData.offsetC = (uint16_t)avgC;
	
	cprintf("Offset - I: 0x%04X; Q: 0x%04X; C: 0x%04X\n",(uint16_t)avgI, (uint16_t)avgQ, (uint16_t)avgC);
	cprintf("ADC Offset Calibration OK\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	ADC_IRQHandler
*	�������ܣ�	ADC�жϷ�����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void ADC_IRQHandler(void)
{
	if(RESET != adc_interrupt_flag_get(ADC0,ADC_INT_FLAG_EOC))	//ADCת�����
	{
		g_stADCData.dataI = adc_regular_data_read(ADC0);
		g_stADCData.dataQ = adc_regular_data_read(ADC1);
		g_stADCData.dataC = adc_regular_data_read(ADC2);
		g_stADCData.count++;
		
		adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);//���ADC�жϱ�־
	}
}
/** ---	End of File ---------------------------------------------------------------------------------  **/ 

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
**                          补偿校准时，采用10Hz采样频率，正常工作时采用1Hz采样频率进行监视。
**							由于频率较低，使用中断方式接收数据
**							使用Timer2按照周期触发采集，ADC产生中断保存数据，在程序中处理数据
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
	uint16_t    offsetI;	//Sin偏移
	uint16_t	offsetQ;	//Cos偏移
	uint16_t    offsetC;	//电流偏移
	uint16_t	count;	//读取计数
	uint16_t    rsvd;	//保留
}tADCDatas;


/** ---	Start of Para define 	---------------------------------------------------------------------  **/
tADCDatas g_stADCData;

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	函数名称：	adc_init
*	函数功能：	初始化ADC采集
*	入口参数：	None
*	返回参数：	None
*	说明：      可使用ADC012，基于DMA传输
*               PC0 - ADC_DQ    IN10
*               PC1 - ADC_DI    IN11
*               PC2 - IV_TEST   IN12
************************************************************/
void ADC_Init(void)
{
	int i;
	
    rcu_periph_clock_enable(RCU_GPIOC);//使能GPIOC的时钟

    rcu_periph_clock_enable(RCU_ADC0);//使能ADC0的时钟 
    rcu_periph_clock_enable(RCU_ADC1);//使能ADC1的时钟
	rcu_periph_clock_enable(RCU_ADC2);//使能ADC2的时钟	

    adc_clock_config(ADC_ADCCK_HCLK_DIV20);//配置ADC的时钟: 168MHz/20 = 8.4MHz, ADC最大时钟40MHz
	 
	gpio_mode_set(GPIOC,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);//模拟输入
 
    adc_channel_length_config(ADC0,ADC_REGULAR_CHANNEL,1);//通道长度配置
    adc_channel_length_config(ADC1,ADC_REGULAR_CHANNEL,1);
	adc_channel_length_config(ADC2,ADC_REGULAR_CHANNEL,1);		
    
	adc_special_function_config(ADC0,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);//关闭ADC扫描模式功能和连续采集功能：SM=0, CTN=0
	adc_special_function_config(ADC1,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);
	adc_special_function_config(ADC2,ADC_SCAN_MODE | ADC_CONTINUOUS_MODE,DISABLE);
	
	adc_discontinuous_mode_config(ADC0,ADC_CHANNEL_DISCON_DISABLE,1);//配置ADC的间断采集模式，DISRC=0

	adc_regular_channel_config(ADC0,0,ADC_CHANNEL_11,ADC_SAMPLETIME_480);//ADC_DI: 采样时间：480/8.4MHz = 57.1us，最大采样频率: 8.4MHz/
    adc_regular_channel_config(ADC1,0,ADC_CHANNEL_10,ADC_SAMPLETIME_480);//ADC_DQ
	adc_regular_channel_config(ADC2,0,ADC_CHANNEL_12,ADC_SAMPLETIME_480);//IV_TEST

    adc_external_trigger_config(ADC0,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_RISING);	//使能ADC0的外部Timer触发，其他ADC禁用触发
	adc_external_trigger_config(ADC1,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_DISABLE);	
	adc_external_trigger_config(ADC2,ADC_REGULAR_CHANNEL,EXTERNAL_TRIGGER_DISABLE);
	
    adc_external_trigger_source_config(ADC0,ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_T4_CH0);//ADC使用了Timer4 CH0进行触发
    
    adc_data_alignment_config(ADC0,ADC_DATAALIGN_RIGHT);//ADC数据校准配置
    adc_data_alignment_config(ADC1,ADC_DATAALIGN_RIGHT);
	adc_data_alignment_config(ADC2,ADC_DATAALIGN_RIGHT);
 
    adc_sync_mode_config(ADC_ALL_REGULAL_PARALLEL);//配置ADC同步模式
	
	adc_oversample_mode_config(ADC0,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	//配置ADC为256倍过采样，输出16bit
	adc_oversample_mode_config(ADC1,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	
	adc_oversample_mode_config(ADC2,ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL256);	
	
	adc_oversample_mode_enable(ADC0);	//使能ADC过采样模式
	adc_oversample_mode_enable(ADC1);
	adc_oversample_mode_enable(ADC2);

    adc_enable(ADC0);//使能ADC0接口
	adc_enable(ADC1);
	adc_enable(ADC2);	

	for(i=0;i<280;i++){};//等待至少14个ADCCLK以保证ADC稳定: 168MHz/8.4MHz*14= 280

    adc_calibration_enable(ADC0);//ADC0对校准复位并重新校准
    adc_calibration_enable(ADC1); 
	adc_calibration_enable(ADC2);		
		
	Timer4_Init();
		
	g_stADCData.offsetI = 0x8000;
	g_stADCData.offsetQ = 0x8000;
	g_stADCData.offsetC = 0;
		
	nvic_irq_enable(ADC_IRQn, 6, 6);
}

/************************************************************
*	函数名称：	ADC_start
*	函数功能：	启动ADC采集
*	入口参数：	period - 数据采集的周期（ms）
*	返回参数：	None
*	说明：      
************************************************************/
void ADC_start(uint16_t period)
{
	g_stADCData.count = 0;
	adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);//清除ADC中断标志
	adc_interrupt_enable(ADC0, ADC_INT_EOC);//使能ADC0中断
	
	Timer4_SetPeriod(period);
	timer_enable(TIMER4);//使能Timer4
}

/************************************************************
*	函数名称：	ADC_stop
*	函数功能：	停止ADC采集
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void ADC_stop(void)
{
	timer_disable(TIMER4);//禁用Timer4
	adc_interrupt_disable(ADC0, ADC_INT_EOC);	//禁用ADC0中断
}

/************************************************************
*	函数名称：	ADC_Test
*	函数功能：	ADC采集测试
*	入口参数：	None
*	返回参数：	None
*	说明：      ADC采样时间：480clk，转换时间12clk，平均次数：256次
*				ADC时钟频率：8.4MHz，因此ADC一次转换需要：(480+12)*256 = 125952clk（约15ms）
************************************************************/
void ADC_Test(void)
{
	uint16_t period = 500;	//采样周期：毫秒
	int iData;
	uint16_t count=0, num=0;	//采样计数
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
	
	ADC_start(period);	//启动采样
	while(ERROR == CS_QueryChar(&ch))
	{
		if(count != g_stADCData.count)	//计数变化，显示数据
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
			
			//INA381A1放大20V/V，检测电阻0.47Ω
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
*	函数名称：	ADC_Sample
*	函数功能：	按照period周期，采样num个数据进行分析
*	入口参数：	period  - 采样周期，单位ms
*				num     - 采样点数
*				pDataI	- 返回的I信号平均值
*				pDataQ  - 返回的Q信号平均值
*	返回参数：	None
*	说明：		
************************************************************/
void ADC_Sample(uint16_t period, uint16_t num, int16_t *pAvgI, int16_t *pAvgQ)
{
    uint32_t sumI=0, sumQ=0;
	int i;
    uint16_t count = 0;		//采样计数
	
	i  = num;
 	ADC_start(period);	//启动采样,按照500ms采样周期
	while(i)
	{
		if(count != g_stADCData.count)	//计数变化，显示数据
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
*	函数名称：	ADC_SampleAll
*	函数功能：	按照period周期，采样num个数据（三通道）进行分析，
*	入口参数：	period  - 采样周期，单位ms
*				num     - 采样点数
*				pDataI	- 返回的I信号平均值
*				pDataQ  - 返回的Q信号平均值
*	返回参数：	None
*	说明：		
************************************************************/
void ADC_SampleAll(uint16_t period, uint16_t num, int16_t *pAvgI, int16_t *pAvgQ, int16_t *pAvgC)
{
    uint32_t sumI=0, sumQ=0, sumC=0;
	int i;
    uint16_t count = 0;		//采样计数	
	
	i  = num;
 	ADC_start(period);	//启动采样,按照500ms采样周期
	while(i)
	{
		if(count != g_stADCData.count)	//计数变化，显示数据
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
*	函数名称：	ADC_OffsetCal
*	函数功能：	对ADC的偏移进行标定
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void ADC_OffsetCal(void)
{
	uint16_t period=500;
	int16_t avgI, avgQ, avgC;
	int iData;
	
    PWM_Enable(0);  //所有通道不使能
	
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
    delay_1ms(2000);    //等待数据稳定：2秒  
	ADC_SampleAll(period, 16, &avgI, &avgQ, &avgC);
	
	g_stADCData.offsetI = (uint16_t)avgI;
	g_stADCData.offsetQ = (uint16_t)avgQ;
	g_stADCData.offsetC = (uint16_t)avgC;
	
	cprintf("Offset - I: 0x%04X; Q: 0x%04X; C: 0x%04X\n",(uint16_t)avgI, (uint16_t)avgQ, (uint16_t)avgC);
	cprintf("ADC Offset Calibration OK\n");
	CS_WaitKey();
}

/************************************************************
*	函数名称：	ADC_IRQHandler
*	函数功能：	ADC中断服务函数
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void ADC_IRQHandler(void)
{
	if(RESET != adc_interrupt_flag_get(ADC0,ADC_INT_FLAG_EOC))	//ADC转换完成
	{
		g_stADCData.dataI = adc_regular_data_read(ADC0);
		g_stADCData.dataQ = adc_regular_data_read(ADC1);
		g_stADCData.dataC = adc_regular_data_read(ADC2);
		g_stADCData.count++;
		
		adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);//清除ADC中断标志
	}
}
/** ---	End of File ---------------------------------------------------------------------------------  **/ 

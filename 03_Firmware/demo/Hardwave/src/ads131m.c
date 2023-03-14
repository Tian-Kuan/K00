/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               ads131m.c
** Latest modified Date:    2022-5-27
** Latest Version:          1.0
** Descriptions:            Used for ADS131M02 ADC Sample Test
**							pingpong DMA is used
**							ADC FRAME Set As 32bit，No CRC(for the 32bit CRC align left, CPU Can't auto calculate) 
**							if CRC is needed, Set ADC Frame As 16bit
**							When DMA，SPI frame is 16bit width，Data is 32bit width
**							When Communication, SPI frame is 16bit width, Data is 24bit->32bit
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-27
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             WuYong
** Modified date:           2022-10-29
** Version:                 1.1
** Descriptions:            将ADS131M从可配置帧宽改为固定16bit帧宽，以减少代码量，使代码更加明晰
**							只支持2通道、16bit传输
**							修改ADC DRDY、RESET引脚配置
**							在SPI配置时，CPU工作在SPI主模式，且使用如下引脚：
**								PB12  - SPI_CS(未用，置为GPIO IN）
**								PC9   - CS(GPIO OUT)
**								PB13  - SPI_SCK, OUT
**								PD14  - SCK(未用，置为GPIO IN)
**								PB14  - SPI_MISO, IN
**								PB15  - SPI_MOSI, OUT
**							在SPI采集数据，自动DMA时，CPU工作在SPI从模式，且使用如下引脚：
**								PB12  - SPI_CS， IN
**								PC9   - CS OUT, Timer2_CH3
**								PB13  - SPI_SCK, IN
**								PD14  - SCK OUT, Timer3_CH2
**								PB14  - SPI_MISO, IN
**								PB15  - MOSI(未用)
**
*********************************************************************************************************/

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"
#include "systick.h"

#include "spi.h"
#include "ads131m.h"
#include "timer.h"
#include "usart.h"
#include "i2s.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
#define ADS131M_TIMEOUT_VALUE		(1000)

#define ADC_RESET_BANK				(GPIOD)
#define ADC_RESET_PIN				(GPIO_PIN_15)

/** ---	Start of Para define 	---------------------------------------------------------------------  **/
//static uint16_t	g_ADSRegMap[NUM_REGISTERS];	//ADS131M寄存器映射

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
//void ADS131M_DefaultReg(void);
//uint16_t ADS131M_EnforceMode(uint16_t data);
ErrStatus ADS131M_TxRxArray(uint16_t *pSrc, uint16_t *pDst, uint16_t size);
ErrStatus SPI1_WaitFlag(uint32_t flag, FlagStatus status);
ErrStatus ADS131M_WriteRegF24(uint8_t addr, uint16_t data);//在复位后，应使用该函数将ADC寄存器改写为16bit帧。

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/** ---------------    ADS131M配置相关    --------------- **/
/************************************************************
*	函数名称：	ADS131M_Config
*	函数功能：	ADS131M使用默认配置
*	入口参数：	None
*	返回参数：	ERROR: 错误；SUCCESS：成功；
*	说明：		WNOTE: 如果使能CRC，而不发送CRC校验码，会导致命令出错，而访问有问题；下次响应为Status
************************************************************/
ErrStatus ADS131M_Config(void)
{
	//对ADS131M进行初始化配置	
	if( SUCCESS != ADS131M_WriteRegF24(CLOCK_ADDRESS, 	//按照24位帧模式配置时钟寄存器
					CLOCK_CHALL_DISABLED |			//禁用所有采集通道以便于使用短帧进行配置
					CLOCK_OSR_2048 |				//采用2048倍过采样，即2kHz采样频率
					CLOCK_PWR_HR))					//使用高精度采集模式
		return ERROR;
	
	if( SUCCESS != ADS131M_WriteRegF24(MODE_ADDRESS,		//按照24位帧模式配置模式寄存器
					(MODE_DEFAULT & (~(MODE_WLENGTH_MASK | MODE_DRDY_FMT_MASK | MODE_RESET_MASK))) | MODE_WLENGTH_16BIT | MODE_DRDY_FMT_NEG_PULSE_FIXED_WIDTH)) //将默认配置改为16bit帧长，DRDY脉冲模式
		return ERROR;
	
	return SUCCESS;
	
	//  不放大，采用默认配置
	//  ADS131M_WriteReg(GAIN1_ADDRESS,		//配置增益寄存器
	//	GAIN1_PGAGAIN0_1 |					//输入参考电压为1.2V，量程为±1.2V，因此不需要放大
	//	GAIN1_PGAGAIN1_1);				
	
	//  配置阈值寄存器和配置寄存器
	//	不进行斩波和阈值监测，不配置

	//  暂不启动采集时钟
	//ADS131M_WriteReg(CLOCK_ADDRESS, 	//配置时钟寄存器
	//	CLOCK_CH0_EN_ENABLED |			
	//	CLOCK_CH1_EN_ENABLED |
	//	CLOCK_OSR_2048 |				//采用2048倍过采样，即2kHz采样频率
	//	CLOCK_PWR_HR);					//使用高精度采集模式
}


/************************************************************
*	函数名称：	ADS131M_Init
*	函数功能：	对ADS131M进行初始化
*	入口参数：	None
*	返回参数：	None
*	说明：		ADC 控制引脚
*				PC6	- DRDY IN	（已经在Timer2_Init中配置为GPIO输入）
*				PD15 - RESET OUT
*				CH0 - I信号，Sin，滞后驱动90°（Y轴）
*				CH1 - Q信号，Cos，与驱动同相（X轴）
************************************************************/
void ADS131M_Init(void)
{
	Timer7_Init();		//初始化ADS131M时钟，该时钟为8.4MHz，要得到8.192MHz，考虑使用I2S时钟。	
	Timer2_Init();		//用于在Timer自动采集模式产生\CS信号
	Timer3_Init();		//用于在Timer自动采集模式产生SCLK信号
	//I2SCLK_Init();	//用于产生I2S时钟，使得ADC时钟为8.192MHz
	SPI1_DMA_Init();	//初始化SPI的DMA
	SPI1_Init();		//初始化SPI
	
	rcu_periph_clock_enable(RCU_SYSCFG);	//用于中断

	//初始化GPIO: DRDY/RESET
	rcu_periph_clock_enable(RCU_GPIOD);//开启GPIOD的时钟
    //gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_8);		//PD8 DRDY IN
    //gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);//配置为输入模式，上拉
    gpio_mode_set(ADC_RESET_BANK, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ADC_RESET_PIN);			//PD15 Reset, 输出, 上拉	
    gpio_output_options_set(ADC_RESET_BANK, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ADC_RESET_PIN);//推挽输出，引脚频率50Mhz
    gpio_bit_reset(ADC_RESET_BANK, ADC_RESET_PIN);//复位有效
	
	delay_1ms(50);	//等待50ms的上电稳定时间
	ADS131M_Reset();//对ADC进行复位
}

/************************************************************
*	函数名称：	ADS131M_Reset
*	函数功能：	对ADS131M进行硬件复位
*	入口参数：	None
*	返回参数：	若复位成功，则返回SUCCESS，否则返回ERROR
*	说明：		
************************************************************/
ErrStatus ADS131M_Reset(void)
{
	uint32_t iTimeOut = 0x0280DE80;	//大约数秒
	uint16_t rxData;
	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//复位前配置SPI为8帧模式以避免复位后访问错误
	
	gpio_bit_reset(ADC_RESET_BANK, ADC_RESET_PIN);	//低电平复位		
	delay_1ms(2);	//复位至少250us，此处为1ms	
	gpio_bit_set(ADC_RESET_BANK, ADC_RESET_PIN);//高电平输出，复位完成	
	//delay_1ms(2);	//等待复位完成
	
	//复位后，需要DRDY变为高电平才能正常通讯
	while(iTimeOut--)
	{
		if(RESET != gpio_input_bit_get(ADC_DRDY_BANK, ADC_DRDY_PIN))
			break;
	}
	
    if(ERROR == ADS131M_SendCmd(OPCODE_NULL, &rxData))	//复位后，读回第一个数据应为ID
        return ERROR;
    
    //检查响应是否为ID
    if(rxData != 0xFF22)    //若响应为0x0011说明收到帧，但是不完整
        return ERROR;
	
	return ADS131M_Config();		//进行初始化配置	
}

/************************************************************
*	函数名称：	ADS131M_Reset
*	函数功能：	对ADS131M进行同步，重新开始采集，并清除内部FIFO数据
*	入口参数：	bWaitDrdy:  等待DRDY变成高电平, 0为不等待，其他为等待
*	返回参数：	None
*	说明：		同步时，应屏蔽所有中断，避免因延迟而复位
*				根据示波器观察，实际同步脉冲约为 2.5us（约420个时钟周期）
*				RESET/SYNC: 1~2047个ADC时钟周期为同步，2048个及以上为复位
************************************************************/
void ADS131M_Sync(uint8_t bWaitDrdy)
{
	int i;
	uint32_t iTimeOut = 0x0280DE80;	//大约数秒
	if(bWaitDrdy)	//在使用边沿触发时，在外部禁用irq；在电平触发时，在内部禁用irq;
		__disable_irq();		//避免中断引起同步时间过长而复位
	gpio_bit_reset(GPIOD, GPIO_PIN_15);	//低电平同步		
	//至少1~2047个时钟(约122ns~249us)，为同步ADC并重新开始采集
	for(i=0;i<100;i++);	//约2.5us（每次循环约4个时钟周期）
	gpio_bit_set(GPIOD, GPIO_PIN_15);//高电平输出，同步完成
	
	//在电平触发的时候，需要等待DRDY变高电平
	//在边沿触发的时候，不需要等待DRDY变高电平
	if(bWaitDrdy)
	{
		__enable_irq();
		while(iTimeOut--)
		{
			if(RESET != gpio_input_bit_get(ADC_DRDY_BANK, ADC_DRDY_PIN))
				break;
		}
	}
}

/************************************************************
*	函数名称：	ADS131M_ReadData
*	函数功能：	读取采集到的数据
*	入口参数：	pDataSt - 数据存储区
*	返回参数：	错误：ERROR；正确：SUCCESS
*	说明：		接收数据为16bit帧
*               采集数据均带符号
************************************************************/
ErrStatus ADS131M_ReadData(adc_channel_data *pDataSt)
{
    uint16_t txBuf[4] = {0};
    uint16_t rxBuf[4] = {0};

    if(ERROR == ADS131M_TxRxArray( txBuf, rxBuf, 4)) 
        return ERROR;
    
    pDataSt->response = rxBuf[0];
	pDataSt->channel0 = (int16_t) rxBuf[1]; 
    pDataSt->channel1 = (int16_t) rxBuf[2];
	pDataSt->crc = rxBuf[3];
	
    return SUCCESS;
}

/************************************************************
*	函数名称：	ADS131M_ReadReg
*	函数功能：	读取寄存器数据
*	入口参数：	addr	- 寄存器地址
*				pRxData - 接收缓冲区
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		
************************************************************/
ErrStatus ADS131M_ReadReg(uint8_t addr, uint16_t *pRxData)
{
	uint16_t opcode;
	
	addr &= 0x3F;
	opcode = OPCODE_RREG | (((uint16_t) addr)<<7);
	
	if(ERROR == ADS131M_SendCmd(opcode, pRxData))	//发操作字
		return ERROR;
	
	if(ERROR == ADS131M_SendCmd(OPCODE_NULL, pRxData))	//接收数据
		return ERROR;
	
	//g_ADSRegMap[addr] = *pRxData;	
	return SUCCESS;
}

/************************************************************
*	函数名称：	ADS131M_WriteReg
*	函数功能：	按照16bit帧格式，写入寄存器数据
*	入口参数：	addr	- 寄存器地址
*				data	- 待写入数据
*				bCheck  - 对写入的数据进行读回校验
*						  ADCSPI_CHECK_YES 	  - (1)需要读回校验
*						  ADCSPI_CHECK_NONE   - (0)不进行读回校验
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		
************************************************************/
ErrStatus ADS131M_WriteReg(uint8_t addr, uint16_t data, uint8_t bCheck)
{
	uint16_t opcode[2];
	uint16_t rxBuf[2];
	
	addr &= 0x3F;
	opcode[0] = OPCODE_WREG | (((uint16_t) addr)<<7);
	opcode[1] = data;
	
	if(ERROR == ADS131M_TxRxArray(opcode, rxBuf, 2))
		return ERROR;
	
	if(bCheck == ADCSPI_CHECK_YES)	//需要进行校验
	{
		if(ERROR == ADS131M_ReadReg(addr, rxBuf))	//回读数据进行确认，按照bcheck要求的帧类型
			return ERROR;
	
		if(rxBuf[0] != data)
			return ERROR;	
	}

	return SUCCESS;
}

/************************************************************
*	函数名称：	ADS131M_WriteRegF24
*	函数功能：	按照24bit帧格式写入寄存器数据
*	入口参数：	addr	- 寄存器地址
*				data	- 待写入数据
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		只在系统启动时使用24bit帧写入，后面会改为16bit帧
************************************************************/
ErrStatus ADS131M_WriteRegF24(uint8_t addr, uint16_t data)
{
	uint16_t opcode[3];
	uint16_t rxBuf[3];
	
	addr &= 0x3F;
	opcode[0] = OPCODE_WREG | (((uint16_t) addr)<<7);
	opcode[1] = data>>8;
	opcode[2] = data<<8;
	
	return ADS131M_TxRxArray(opcode, rxBuf, 3);
}

/************************************************************
*	函数名称：	ADS131M_SoftReset
*	函数功能：	发复位命令进行软件复位
*	入口参数：	None
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		
************************************************************/
ErrStatus ADS131M_SoftReset(void)
{
    uint16_t rxBuf[CHANNEL_COUNT+1+1];  //通道数+CRC+OPCODE
    uint16_t txBuf[CHANNEL_COUNT+1+1] = {0};    
    
    txBuf[0] = OPCODE_RESET;
    
	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//复位前配置SPI为8帧模式以避免复位后访问错误
	
	if(ERROR == ADS131M_TxRxArray(txBuf, rxBuf, CHANNEL_COUNT+1+1))  //按照16bit接收，发送复位指令帧
        return ERROR;
    
    delay_1ms(1);   //等待复位完成
    
    if(ERROR == ADS131M_SendCmd(OPCODE_NULL, rxBuf))	//复位后，读回第一个数据应为ID
        return ERROR;
    
    //检查响应是否为ID
    if(rxBuf[0] != 0xFF22)    //若响应为0x0011说明收到帧，但是不完整
        return ERROR;
    
	return ADS131M_Config();		//进行初始化配置	
}

/************************************************************
*	函数名称：	ADS131M_LockReg
*	函数功能：	锁定寄存器数据，防止被修改，以进行专注的数据采集
*	入口参数：	None
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		按照16bit数据帧
************************************************************/
ErrStatus ADS131M_LockReg(void)
{
    uint16_t rxData;
    
    if(ERROR == ADS131M_SendCmd(OPCODE_LOCK,&rxData))
        return ERROR;
    
    if(ERROR == ADS131M_ReadReg(STATUS_ADDRESS, &rxData))
        return ERROR;
    
    if(!SPI_LOCKED(rxData))
       return ERROR;
    return SUCCESS;    
}

/************************************************************
*	函数名称：	ADS131M_UnlockReg
*	函数功能：	解锁寄存器，允许对寄存器进行修改
*	入口参数：	None
*	返回参数：	错误：ERROR；成功：SUCCESS
*	说明：		只按照16bits帧长
************************************************************/
ErrStatus ADS131M_UnlockReg(void)
{
    uint16_t rxData;
    
    if(ERROR == ADS131M_SendCmd(OPCODE_UNLOCK,&rxData))
        return ERROR;
    
    if(ERROR == ADS131M_ReadReg(STATUS_ADDRESS, &rxData))
        return ERROR;
    
    if(SPI_LOCKED(rxData))
       return ERROR;
    return SUCCESS;  
}

/************************************************************
*	函数名称：	SPI1_WaitFlag
*	函数功能：	等待SPI1的某个状态出现
*	入口参数：	flag   - 等待的事件
*				status - 事件的状态（SET/RESET)
*	返回参数：	若超时，ERROR,否则返回SUCCESS
*   说明：      
************************************************************/
ErrStatus SPI1_WaitFlag(uint32_t flag, FlagStatus status)
{
	uint32_t iTimeout = ADS131M_TIMEOUT_VALUE;
	while(status != spi_i2s_flag_get(SPI1, flag))  //等待接收到数据
	{
		if(0 == iTimeout--) //超时
			return ERROR;
	}
	return SUCCESS;
}

/************************************************************
*	函数名称：	ADS131M_SendCmd
*	函数功能：	向ADS131M发送一个命令
*	入口参数：	opcode	- 待发送的命令码，只可为如下命令：
*						NULL	无操作
*						STANDBY	待机
*						WAKEUP	唤醒*		
*				pRxData  - 接收的16bit数据
*	返回参数：	错误，返回ERROR，否则SUCCESS
*	说明：		其他命令应使用专用函数:
*						RESET	ADS131M_SoftReset
*						LOCK	ADS131M_LockReg
*						UNLOCK	ADS131M_UnlockReg
*						RREG	ADS131M_ReadReg
*						WREG	ADS131M_WriteReg
************************************************************/
ErrStatus ADS131M_SendCmd(uint16_t opcode, uint16_t *pRxData)
{
	return ADS131M_TxRxArray( &opcode, pRxData, 1);
}

/************************************************************
*	函数名称：	ADS131M_TxRxArray
*	函数功能：	通过SPI1收发一组数据
*	入口参数：	pSrc    - 待发送的数据空间
*				pDst	- 待接收的数据空间
*				size	- 待接收的数据个数
*	返回参数：	若超时，返回ERROR,成功SUCCESS
*   说明：      默认按照16bit帧长处理   
************************************************************/
ErrStatus ADS131M_TxRxArray(uint16_t *pSrc, uint16_t *pDst, uint16_t size)
{
	uint16_t i;
	
    *pDst = spi_i2s_data_receive(SPI1);        //读空接收寄存器
	
	ADS131M_CSLow();	//使能片选
	
	for(i=0; i<size; i++)
	{
		spi_i2s_data_transmit(SPI1, pSrc[i]);   //启动一次发收//	
		if(ERROR == SPI1_WaitFlag(SPI_FLAG_RBNE, SET))
		{
			ADS131M_CSHigh();	//片选结束
			return ERROR;
		}
		*pDst++ = spi_i2s_data_receive(SPI1);	
	}
	
	ADS131M_CSHigh();	//片选禁用
    return SUCCESS;	
}


/** ---------------    ADS131M采集相关    --------------- **/
/************************************************************
*	函数名称：	ADS131M_StartSample
*	函数功能：	启动ADC的采集
*	入口参数：	
*	返回参数：	None
*   说明：      可进行DMA采集或者轮询方式采集
************************************************************/
void ADS131M_StartSample(void)
{
	ADS131M_WriteReg(CLOCK_ADDRESS, 	//配置时钟寄存器，使能采集通道
		CLOCK_CH0_EN_ENABLED |			
		CLOCK_CH1_EN_ENABLED |
		CLOCK_OSR_2048 |				//采用2048倍过采样，即2kHz采样频率
		CLOCK_PWR_HR,					//使用高精度采集模式	
		ADCSPI_CHECK_NONE);
	ADS131M_LockReg();					//锁定寄存器，不允许修改	
	
	//硬件同步
	ADS131M_Sync(1);					//重新进行同步，并且等待DRDY变高电平
}

/************************************************************
*	函数名称：	ADS131M_StopSample
*	函数功能：	停止ADC的采集
*	入口参数：	None
*	返回参数：	None
*   说明：      
************************************************************/
void ADS131M_StopSample(void)
{
	ADS131M_UnlockReg();				//解锁寄存器访问
	ADS131M_WriteReg(CLOCK_ADDRESS, 	//配置时钟寄存器
		CLOCK_CHALL_DISABLED |			//禁止通道采集
		CLOCK_OSR_2048 |				//采用2048倍过采样，即2kHz采样频率
		CLOCK_PWR_HR,					//使用高精度采集模式		
		ADCSPI_CHECK_NONE);
}

/************************************************************
*	函数名称：	ADS131M_StartDMA
*	函数功能：	启动ADC的DMA采集
*	入口参数：	None
*	返回参数：	None
*   说明：      需查询DMA数据状态以获取数据
************************************************************/
void ADS131M_StartDMA(void)
{
	__disable_irq();		//防止被中断而导致第一次DMA丢失
	ADS131M_StartSample();
	SPI1_DMAStart();
	
	ADS131M_Sync(0);		//使用下降沿触发，无需等待
	EXADC_TimerDrdyEnable();
	__enable_irq();
}

/************************************************************
*	函数名称：	ADS131M_StartDMA
*	函数功能：	启动ADC的DMA采集
*	入口参数：	None
*	返回参数：	None
*   说明：      需查询DMA数据状态以获取数据
************************************************************/
void ADS131M_StopDMA(void)
{
	SPI1_DMAStop();
	ADS131M_StopSample();
}

/************************************************************
*	函数名称：	ADS131M_QuerySmpNum
*	函数功能：	采用查询采集方式采集指定数量的数据
*	入口参数：	pBuf	- 数据缓存
*				size	- 采集数量（CH0/CH1总数量）
*	返回参数：	None
*	说明：		输出CH0放在低半地址区，CH1放在高半地址区
*				由于第一个采集数据误差较大，会丢弃第一个
************************************************************/
ErrStatus ADS131M_QuerySmpNum(uint16_t *pBuf, uint8_t size)
{
	adc_channel_data stData;
	uint8_t iCnt, iInc;
	uint16_t *pBuf1;
	ErrStatus iStatus = SUCCESS;

	iCnt = 0;
	size >>= 1;	
	pBuf1 = pBuf + size;
	iInc = 0;	//增量，用于丢弃第一点
	
	ADS131M_StartSample();

	while(iCnt<=size)	//第一点会被丢弃，故多取一个
	{
		if(RESET == gpio_input_bit_get(ADC_DRDY_BANK, ADC_DRDY_PIN))	//数据到达
		{
			if(ERROR == ADS131M_ReadData(&stData))
			{
				pBuf[iCnt]  = 0xABCD;
				pBuf1[iCnt] = 0xABCD;
				iStatus = ERROR;
			}
			else
			{
				pBuf[iCnt] = (uint16_t) stData.channel0;
				pBuf1[iCnt] = (uint16_t) stData.channel1;
			}			
			iCnt += iInc;
			iInc=1;
		}
	}		
	
	ADS131M_StopSample();
	return iStatus;
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

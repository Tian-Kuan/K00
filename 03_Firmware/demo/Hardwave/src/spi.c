/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               spi.c
** Latest modified Date:    2022-5-25
** Latest Version:          1.0
** Descriptions:            Used for Fram Test(SPI0) & ADC Test(SPI1)
**							FM25L16B(2Kx8), SPI0, Max. 20MHz
**                          PCLK2: AHB/2 = 84MHz; SPI CLK: 84MHz/8 = 10.5MHz 
**                          PA4 - NSS
**                          PA5 - SCK
**                          PA6 - MISO
**                          PA7 - MOSI
**
**                          ADS131M02, SPI1, Max. 25MHz
**                          PCLK1: AHB/4 = 42MHz; SPI CLK: 42MHz/4 = 10.5MHz 
**                          PB12 - NSS		<-> PC9  - Timer2_CH3
**                          PB13 - SCK		<-> PD14 - Timer3_CH2
**                          PB14 - MISO
**                          PB15 - MOSI
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-25
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             WuYong
** Modified date:           2022-10-28
** Version:                 1.1
** Descriptions:            SPI操作从8bit改为16bit帧
**							增加了自动接收时的 单线接收从模式，用于数据自动采集
**							增加了自动接收模式 和 手动配置模式的切换功能
*********************************************************************************************************/

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"
#include "systick.h"

#include "spi.h"
#include "timer.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/

/** ---	Start of Para define 	---------------------------------------------------------------------  **/
uint16_t g_SPI1DMARxBufPi[SPI1_DMABUF_SIZE];	//Used for PingPong
uint16_t g_SPI1DMARxBufPo[SPI1_DMABUF_SIZE];	//Used for PingPong

//const static uint32_t g_SPI1DMATxBuf[SPI1_DMAONE_SIZE] = {0};

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/


/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	函数名称：	SPI0_Init
*	函数功能：	SPI0初始化，用于FRAM访问
*	入口参数：	None
*	返回参数：	None
*   说明：      (1):PA7 - MOSI
*	            (2):PA6 - MISO
*	            (3):PA5 - SCK
*	            (4):PA4 - NSS：	软件NSS模式：不使用
*					若主机硬件NSS模式：为NSS输出，NSSDRV=1时，为单主机模式
*                   经测试，只要使能SPI，NSS在硬件模式下输出保持为低电平。
************************************************************/
void SPI0_Init(void)
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
	
    //设置PA5/PA6/PA7为SPI的SCK/MISO/MOSI
    gpio_af_set(GPIOA, GPIO_AF_5, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
    //设置PA4位IO输出，用于软件控制NSS
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);//输出模式，不上拉
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);//推挽输出，引脚频率50Mhz
    gpio_bit_set(GPIOA, GPIO_PIN_4);    //高电平输出
     
	spi_i2s_deinit(SPI0);		//复位SPI0配置
	
	 /* SPI0配置 */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    spi_init_struct.device_mode          = SPI_MASTER;				//主机模式
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;		//设置SPI的数据大小:SPI发送接收8位帧结构
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;  //时钟低电平，第一边沿
    spi_init_struct.nss                  = SPI_NSS_SOFT;			//使用软件控制，可防止中断影响
    spi_init_struct.prescale             = SPI_PSC_8;               //分频后为：10.5MHz
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
	
	spi_crc_polynomial_set(SPI0, 0x07);	//CRC多项式: CRC8 ITU ROHC
//	spi_crc_on(SPI0);		//使能CRC
	
	spi_enable(SPI0);		//使能SPI0	
}

#define SPI1_MFD_SRB_MSK	(~(SPI_CTL0_BDEN | SPI_CTL0_SWNSSEN | SPI_CTL0_MSTMOD))
#define SPI1_MODE_SRB		(SPI_CTL0_BDEN)				//单线双向模式，从模式
#define SPI1_MODE_MFD		(SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN)	//双线单向模式，主模式, 软件片选
/************************************************************
*	函数名称：	SPI1_Init
*	函数功能：	SPI1初始化，用于ADC访问
*	入口参数：	None
*	返回参数：	None
*   说明：      (1):PB15 - MOSI
*	            (2):PB14 - MISO
*	            (3):PB13 - SCK
*	            (4):PB12 - NSS：	软件NSS模式：不使用
*					若主机硬件NSS模式：为NSS输出，NSSDRV=1时，为单主机模式
*                   经测试，只要使能SPI，NSS在硬件模式下输出保持为低电平。
*				ADS131M02对SPI输入时序有严格要求，因此SPI也不可工作在TI模式下
*				由于NSS不能自动产生，需要程序控制，DMA较为复杂，若要最高效率，可考虑Timer驱动NSS和CLK的从机双向模式：SRB模式
*				当前情况下，要使用DMA，可以考虑双中断方式：① DRDY中断，使能片选，启动SPI数据读取；② DMA读取到数据，发中断，在中断中停止片选
*				也可以考虑在中断中进行延时，重新使能片选，但是至少要保证片选信号20ns以上的高电平（本例采取该方式）
*				WNOTE：若片选长时间保持低电平，ADS131M的DRDY将不会发出，直到SPI超时复位；可能使能CRC可解决此问题，但需要尝试。
*
*				全双工主机模式MFD：
*					MSTMOD = 1, 主机模式		RO = 0，全双工模式
*					BDEN = 0, 双线单向模式		BDOEN = 0，双线时无效
*					NSSDRV = 0, 驱动NSS，		SWNSSEN = 1，软件控制
*					SWNSS = 1，避免进入多主
*				双向线连接从机接收模式SRB：
*					MSTMOD = 0, 从机模式		RO = 0，单线时无效
*					BDEN = 1, 单线双向模式		BDOEN = 0, 只工作于接收模式
*					NSSDRV = 0, 接收NSS，		SWNSSEN = 0，硬件控制
*					SWNSS = 1，无意义
************************************************************/
void SPI1_Init(void)
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI1);
	
    //设置PB13/PB14/PB15为SPI的SCK/MISO/MOSI
    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	gpio_bit_reset(GPIOB, GPIO_PIN_15);	//在设置MOSI为GPIO输出的时候，保证输出的数据为低电平
	
    //设置PB12为IO输出，用于软件控制NSS
	gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12);             			//SPI CS可选功能默认为SPI功能,当前未生效
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//输出模式，不上拉
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);//推挽输出，引脚频率50Mhz
    gpio_bit_set(GPIOB, GPIO_PIN_12);    //高电平输出
        
	spi_i2s_deinit(SPI1);		//复位SPI1配置
	
	 /* SPI1配置 */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    spi_init_struct.device_mode          = SPI_MASTER;				//主机模式
    spi_init_struct.frame_size           = SPI_FRAMESIZE_16BIT;		//设置SPI的数据大小:SPI发送接收16位帧结构
	//spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;  //时钟低电平，第二边沿接收， SPI MODE 1
    spi_init_struct.nss                  = SPI_NSS_SOFT;			//使用软件控制，可防止中断影响
    spi_init_struct.prescale             = SPI_PSC_4;               //分频后为：10.5MHz
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);
	spi_nss_output_enable(SPI1);		//使能NSS输出，软件模式下，该位无效
	
	//如果加入CRC，在DMA时，会自动产生CRC数据和进行CRC校验
	//在其他操作时，发送完最后一个字后，需手动触发
//#ifdef ENABLE_CRC_IN	
//	spi_crc_polynomial_set(SPI1, 0x1021);	//CRC多项式: CCITT CRC
//	spi_crc_on(SPI1);		//使能CRC
//#endif

	spi_enable(SPI1);		//使能SPI1	
}

/************************************************************
*	函数名称：	EXADC_SpiMaster_Start
*	函数功能：	外部ADC工作在主SPI模式，内部SPI工作在从机模式
*	入口参数：	None
*	返回参数：	None
*   说明：      
************************************************************/
void EXADC_SpiMaster_Start(void)
{
	spi_disable(SPI1);	//禁用SPI，以进行配置	
	
	//进入 双向线连接从机接收模式SRB
	SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & SPI1_MFD_SRB_MSK) | SPI1_MODE_SRB;
	spi_nss_output_disable(SPI1);	
	
	//将SPI的\CS改为SPI功能，将MOSI改为GPIO输出
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);	//SPI CS改为可选的默认SPI功能 
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);//SPI MOSI改为GPIO输出 
	
	spi_enable(SPI1);	//重新启用SPI1
}

/************************************************************
*	函数名称：	EXADC_SpiMaster_Stop
*	函数功能：	外部ADC工作在从SPI模式，内部SPI工作在主机模式
*	入口参数：	None
*	返回参数：	None
*   说明：      
************************************************************/
void EXADC_SpiMaster_Stop(void)
{
	spi_disable(SPI1);	//禁用SPI，以进行配置	
	
	//将SPI的/CS改为GPIO输出，MOSI改为SPI功能
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_12);	//SPI /CS改为GPIO输出、上拉
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);		//SPI MOSI改为可选的默认SPI功能 
	
	//进入全双工主机模式MFD
	SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & SPI1_MFD_SRB_MSK) | SPI1_MODE_MFD;
	spi_nss_output_enable(SPI1);
	
	spi_enable(SPI1);	//重新启用SPI1
}


/************************************************************
*	函数名称：	SPI1_DMA_Init
*	函数功能：	SPI1 DMA初始化，用于ADC数据的收发
*	入口参数：	None
*	返回参数：	None
*   说明：      DMA0：CH3 - SPI1_RX
*					  CH4 - SPI1_TX
*				SPI在DMA模式下时，需要将片选配置为自动模式
************************************************************/
void SPI1_DMA_Init(void)
{
	dma_multi_data_parameter_struct dma_init_struct;//初始化使用结构体

    rcu_periph_clock_enable(RCU_DMA0);//使能DMA的时钟
	
	//配置 SPI1_RX DMA
    dma_deinit(DMA0, DMA_CH3);//复位DMA通道CH3寄存器
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;				//外设到存储器模式
	dma_init_struct.critical_value = DMA_FIFO_2_WORD;				//FIFO达到2个字（8字节）时写入MEM
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;		//循环传输模式：用于PingPong传输
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;				//超高优先级
	
	dma_init_struct.number = 0;				    					//数据传输量，按照16bit，暂不接收数据
	
    dma_init_struct.memory0_addr = (uint32_t) g_SPI1DMARxBufPi; 	//DMA 存储器0地址
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//存储器增量模式
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//存储器宽度32bit
	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_SINGLE;	//单次传输，FIFO阈值到达后进行2个单次传输
	
    dma_init_struct.periph_addr = (uint32_t) &SPI_DATA(SPI1);		//DMA外设地址
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//外设非增量模式
    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_16BIT;			//外设数据长度：16位
	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//单次传输	

    dma_multi_data_mode_init(DMA0, DMA_CH3, &dma_init_struct);		//存储器突发多次传输	
    dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI0);	//DMA外设通道选择：SPI1_RX
	dma_switch_buffer_mode_enable(DMA0,DMA_CH3,ENABLE);				//使能存储器切换模式

	//配置 SPI1_TX DMA
//    dma_deinit(DMA0, DMA_CH4);	//复位DMA通道CH4寄存器
//    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;				//存储器到外设模式
//	dma_init_struct.critical_value = DMA_FIFO_1_WORD;				//外设模式，1字传输
//	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;		//禁用循环传输模式
//    dma_init_struct.priority = DMA_PRIORITY_HIGH;					//高优先级，低于接收DMA
//	
//	dma_init_struct.number = 0;				    //数据传输量，按照外设16bit，需*2，暂不发送数据
//	
//    dma_init_struct.memory0_addr = (uint32_t) g_SPI1DMATxBuf; 		//DMA发送缓冲
//    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//存储器增量模式
//	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//存储器宽度32bit
//	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_SINGLE;	//1个字进行一次突发传输
//	
//    dma_init_struct.periph_addr = (uint32_t) &SPI_DATA(SPI1);		//DMA外设地址
//    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//外设非增量模式
//    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_16BIT;			//外设数据长度：16位
//	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//单次突发传输	

//    dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);		//存储器突发多次传输	
//    dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);	//DMA外设通道选择：SPI1_TX
}


//仅用于需要发送时
/************************************************************
*	函数名称：	SPI1_DMAReadOnce
*	函数功能：	使用DMA启动读取SPI1一组数据
*	入口参数：	None
*	返回参数：	None
*   说明：      发送3个字的数据
************************************************************/
//void SPI1_DMAReadOnce(void)
//{
//	dma_interrupt_flag_clear(DMA0,DMA_CH4,DMA_INT_FLAG_FTF);		//需要清除传输完成中断，方可
//	dma_transfer_number_config(DMA0,DMA_CH4,SPI1_DMAONE_SIZE*2);	//重置发送数量	
//	dma_channel_enable(DMA0, DMA_CH4);	//使能SPI1 DMA通道的发送
//}

//用于测试TI模式，由于ADS131M对SPI时序的严格要求，TI模式不可用
//void SPI1_TIEnable(uint8_t isTI)
//{	
//	spi_disable(SPI1);	//禁用SPI，以进行配置	
//	
//	if(isTI)
//	{
//		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12);	//配置PB12为片选
//		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
//		spi_ti_mode_enable(SPI1);
//	}
//	else
//	{
//		gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//输出模式，不上拉
//		gpio_bit_set(GPIOB, GPIO_PIN_12);    //高电平输出
//		spi_ti_mode_disable(SPI1);
//	}

//	spi_enable(SPI1);	//重新启用SPI1	
//}

/************************************************************
*	函数名称：	SPI1_DMAStart
*	函数功能：	使能DMA
*	入口参数：	None
*	返回参数：	None
*   说明：      
*   注意：
*   			WNOTE: 将CPU的NSS配置为硬件模式，那么NSS在SPI使能时会一直为低
************************************************************/
void SPI1_DMAStart(void)
{	
	//配置DMA
	dma_interrupt_flag_clear(DMA0,DMA_CH3,DMA_INT_FLAG_FTF);								//需要清除传输完成中断，方可
	dma_transfer_number_config(DMA0,DMA_CH3, SPI1_DMABUF_SIZE);								//重新配置传输数据量, 每次传输16bit
	dma_switch_buffer_mode_config(DMA0,DMA_CH3,(uint32_t) g_SPI1DMARxBufPo,DMA_MEMORY_0);	//选择空间0为初始MEM存储区

	spi_i2s_data_receive(SPI1);	//读空SPI1的接收BUF;
	
	dma_channel_enable(DMA0, DMA_CH3);	 //使能SPI1 DMA通道的接收
	spi_dma_enable(SPI1,SPI_DMA_RECEIVE);//使能SPI1接收DMA
	//spi_dma_enable(SPI1,SPI_DMA_TRANSMIT);//使能SPI1发送DMA
	
	EXADC_SpiMaster_Start();	//更改SPI配置
	EXADC_Timer_Start();		//更改Timer配置并启动
}

/************************************************************
*	函数名称：	SPI1_DMAStop
*	函数功能：	禁用DMA
*	入口参数：	None
*	返回参数：	None
*   说明：      
************************************************************/
void SPI1_DMAStop(void)
{
	EXADC_Timer_Stop();		//还原Timer配置并停止
	EXADC_SpiMaster_Stop();	//还原SPI配置
	
//	spi_dma_disable(SPI1,SPI_DMA_TRANSMIT);	//禁用SPI1发送DMA
	spi_dma_disable(SPI1,SPI_DMA_RECEIVE);	//禁用SPI1接收DMA
	dma_channel_disable(DMA0, DMA_CH3);		//禁用SPI1 DMA通道的接收，并重新初始化传输参数
	
	//将CS修改为GPIO控制模式
//	spi_disable(SPI1);	//禁用SPI，以进行配置
//	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//输出模式，不上拉
//    gpio_bit_set(GPIOB, GPIO_PIN_12);    //高电平输出
//	SPI_CTL0(SPI1) |= SPI_NSS_SOFT;	//将NSS改为软件模式
//	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//普通通讯采用8bit模式
//	spi_enable(SPI1);	//重新启用SPI1
}

/************************************************************
*	函数名称：	SPI1_GetDMABufAddr
*	函数功能：	获取DMA缓冲地址
*	入口参数：	mem		- 空间编号
*	返回参数：	空间地址
*   说明：      
************************************************************/
uint32_t SPI1_GetDMABufAddr(uint32_t mem)
{
	switch(mem)
	{
		case DMA_MEMORY_0:
			return (uint32_t) g_SPI1DMARxBufPi;
		case DMA_MEMORY_1:
			return (uint32_t) g_SPI1DMARxBufPo;
		default:
			return 0;
	}
}

/************************************************************
*	函数名称：	FRAM_Transmit
*	函数功能：	通过SPI0发送一组数据
*	入口参数：	pData   - 待发送的数据空间
*               size    - 待发送的数据数量
*               timeout - 超时检查次数
*	返回参数：	若超时，返回ERROR，否则返回SUCCESS
*   说明：      
************************************************************/
ErrStatus FRAM_Transmit( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
    while(size--)
    {
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //等待发送寄存器为空
        {
            if(0 == iTimeout--) //超时
                return ERROR;
        }
        spi_i2s_data_transmit(SPI0, *pData++);
    }
    
    //等待发送结束
    iTimeout = timeout;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //等待发送寄存器为空
    {
        if(0 == iTimeout--) //超时
            return ERROR;
    }
    iTimeout = timeout;
    while(RESET != spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)) //等待总线收发完毕
    {
        if(0 == iTimeout--) //超时
            return ERROR;
    }
    
    return SUCCESS;
}

/************************************************************
*	函数名称：	FRAM_TransmitCrc
*	函数功能：	通过SPI0发送一组带CRC校验的数据
*	入口参数：	pData   - 待发送的数据空间（含CRC）
*               size    - 待发送的数据数量（含CRC）
*               timeout - 超时检查次数
*	返回参数：	若超时，返回ERROR，否则返回SUCCESS
*   说明：      
************************************************************/
ErrStatus FRAM_TransmitCrc( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
	ErrStatus iStatus = SUCCESS;
	
	spi_disable(SPI0);
	spi_crc_on(SPI0);	//只能在SPI禁用时，修改CRC使能状态
	spi_enable(SPI0);
	
    while(--size)
    {
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //等待发送寄存器为空
        {
            if(0 == iTimeout--) //超时
			{
				iStatus = ERROR;
				goto ERROR;
			}
        }
        spi_i2s_data_transmit(SPI0, *pData++);
		if(size==1)	//最后一个数据，准备发送CRC
			spi_crc_next(SPI0);
    }
    
    //等待发送结束
    iTimeout = timeout;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //等待发送寄存器为空
    {
        if(0 == iTimeout--) //超时
		{
			iStatus = ERROR;
			goto ERROR;
		}
    }
	
    iTimeout = timeout;
    while(RESET != spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)) //等待总线收发完毕：CRC已发送
    {
        if(0 == iTimeout--) //超时
		{
			iStatus = ERROR;
			goto ERROR;
		}
    }
	
	*pData = spi_crc_get(SPI0,SPI_CRC_TX);	//将CRC校验情况保存
    spi_crc_error_clear(SPI0);				//清除接收校验错误
	ERROR:
	spi_disable(SPI0);
	spi_crc_off(SPI0);
	spi_enable(SPI0);
    return iStatus;
}

/************************************************************
*	函数名称：	FRAM_Receive
*	函数功能：	通过SPI0接收一组数据
*	入口参数：	pData   - 待发送的数据空间
*               size    - 待发送的数据数量
*               timeout - 超时检查次数
*	返回参数：	若超时，返回ERROR，否则返回SUCCESS
*   说明：      
************************************************************/
ErrStatus FRAM_Receive( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
    
    *pData = (uint8_t) spi_i2s_data_receive(SPI0);        //读空接收寄存器
    while(size--)
    {
        spi_i2s_data_transmit(SPI0, 0);   //启动一次发收
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))  //等待接收到数据
        {
            if(0 == iTimeout--) //超时
                return ERROR;
        }
        *pData++ = (uint8_t) spi_i2s_data_receive(SPI0);
    }
    return SUCCESS;
}

/************************************************************
*	函数名称：	FRAM_ReceiveCrc
*	函数功能：	通过SPI0接收一组数据，并进行CRC8校验
*	入口参数：	pData   - 待接收的数据空间（含CRC字节）
*               size    - 待接收的数据数量（含CRC字节）
*               timeout - 超时检查次数
*	返回参数：	若超时，返回ERROR，否则返回SUCCESS
*   说明：      
************************************************************/
ErrStatus FRAM_ReceiveCrc( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
	ErrStatus iStatus = SUCCESS;
    
    *pData = (uint8_t) spi_i2s_data_receive(SPI0);        //读空接收寄存器

	spi_disable(SPI0);
	spi_crc_on(SPI0);	//只能在SPI禁用时，修改CRC使能状态
	spi_enable(SPI0);
	
    while(--size)
    {
        spi_i2s_data_transmit(SPI0, 0);   //启动一次发收
		if(size == 1)	//最后一个数据，准备接收CRC校验字节
			spi_crc_next(SPI0);
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))  //等待接收到数据
        {
            if(0 == iTimeout--) //超时
			{
                iStatus = ERROR;
				goto ERROR;
			}
        }
        *pData++ = (uint8_t) spi_i2s_data_receive(SPI0);
    }
	//接收并校验CRC
	iTimeout = timeout;
	while(SET == spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS))
	{
		if(0 == iTimeout--)
		{
			iStatus = ERROR;
			goto ERROR;
		}
	}
	
	if(SET == spi_i2s_flag_get(SPI0, SPI_FLAG_CRCERR))//CRC校验错误
	{
		spi_crc_error_clear(SPI0);
		iStatus = ERROR;
	}
	else
		*pData = spi_crc_get(SPI0,SPI_CRC_RX);	//最后一个为CRC校验
	
	ERROR:
	spi_disable(SPI0);
	spi_crc_off(SPI0);
	spi_enable(SPI0);
    return iStatus;
}


/** ---	End of File ---------------------------------------------------------------------------------  **/ 

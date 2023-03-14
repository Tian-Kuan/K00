/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               usart.c
** Latest modified Date:    2022-5-24
** Latest Version:          1.0
** Descriptions:            Used for UART0(Debug) & UART1(RS232) Init & Communication
**							本程序直接使用查询模式进行通讯，比较费时，但对于测试来说是可用的。
**							在应用程序中，应使用DMA进行发送；如果对通讯的实时性要求不高，也可使用DMA进行接收
**							若对通讯实时性要求较高，对接收应考虑使用中断方式进行译码
**							芯片的USART最高通讯速率可达9Mbps，当使用≥1MBps波特率时，建议使用DMA进行接收。
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-24
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
#include "usart.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

static FlagStatus g_Usart0DmaSending = RESET;   //USART0的DMA发送状态，处于发送状态时，不可进行新一轮发送

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	函数名称：	Usart0_Init
*	函数功能：	调试用串口0初始化（DEBUG）
*	入口参数：	baud - 待设定波特率
*	返回参数：	None
*	说明：		TX-PA9		RX-PA10
************************************************************/
void Usart0_Init(unsigned int baud)
{
    rcu_periph_clock_enable(RCU_GPIOA);//使能时钟
    rcu_periph_clock_enable(RCU_USART0);//使能串口0的时钟

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    //PA9	TXD
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_9);//推挽输出，引脚频率25Mhz
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);//复用功能，上拉

    //PA10	RXD
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_10);//推挽输出，引脚频率25Mhz
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);//复用功能，上拉

    usart_deinit(USART0);

    usart_baudrate_set(USART0, baud);//设置波特率

    usart_parity_config(USART0,USART_PM_NONE);//奇偶校验

    usart_word_length_set(USART0, USART_WL_8BIT);//字长

    usart_stop_bit_set(USART0,USART_STB_1BIT);//停止位

    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);

    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);

    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);//发送模式使能

    usart_receive_config(USART0, USART_RECEIVE_ENABLE);//接收模式使能

    usart_enable(USART0);//使能串口
}

/************************************************************
*	函数名称：	Usart0_DMA_Init
*	函数功能：	调试用串口0 DMA初始化（DEBUG）: 发送DMA初始化
*	入口参数：	
*	返回参数：	None
*	说明：		DMA1(PERI4)：CH5 - USART0_RX(Debug);  CH7 - USART0_TX(Debug);
*               专用于3个Uint32数据，只发送其中一个Uint16数据，递增为6
************************************************************/
void Usart0_DMA_Init(void)
{
	dma_multi_data_parameter_struct dma_init_struct;//初始化使用结构体

    rcu_periph_clock_enable(RCU_DMA1);//使能DMA1的时钟
	
	//配置 USART0_TX DMA
    dma_deinit(DMA1, DMA_CH7);	//复位DMA通道CH4寄存器
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;				//存储器到外设模式
	dma_init_struct.critical_value = DMA_FIFO_4_WORD;				//FIFO 4字空时，存储器传一次
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;		//禁用循环传输模式
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;					//中优先级，低于ADC采集DMA
	
	dma_init_struct.number = 0;				                        //数据传输量，4*4=16的倍数，暂不发送数据
	
    dma_init_struct.memory0_addr = (uint32_t) 0; 		            //DMA发送缓冲，暂不指定地址
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//存储器增量模式
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//存储器宽度16bit
	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_4_BEAT;	//4个32bit进行一次突发传输
	
    dma_init_struct.periph_addr = (uint32_t) &USART_DATA(USART0);	//DMA外设地址
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//外设非增量模式
    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_8BIT;			//外设数据长度：8位
	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//单次突发传输	

    dma_multi_data_mode_init(DMA1, DMA_CH7, &dma_init_struct);		//存储器突发多次传输	
    dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI4);	//DMA1外设通道选择： USART0_TX 
    
    dma_interrupt_flag_clear(DMA1,DMA_CH7,DMA_INT_FLAG_FTF);	    //清除传输完成中断
    dma_interrupt_enable(DMA1,DMA_CH7,DMA_CHXCTL_FTFIE);            //使能DMA发送完成中断
    nvic_irq_enable(DMA1_Channel7_IRQn, 2, 2);                              //中断优先级2，中断子优先级2
    
    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);           //使能串口发送DMA    
}

/************************************************************
*	函数名称：	Debug_DMASend
*	函数功能：	通讯用串口0通过DMA发送一组数据
*	入口参数：	bufAddr - 待发送数据缓存地址
*               len     - 待发送数据的字节长度（必须为16的整数倍）
*	返回参数：	发送成功，返回SUCCESS；否则返回ERROR
*	说明：		
************************************************************/
ErrStatus Debug_DMASend(uint32_t bufAddr, uint16_t len)
{
    if((len&0xF) || g_Usart0DmaSending==SET)         //必须为16的整数倍，且当前不可有数据发送
        return ERROR;
    
    g_Usart0DmaSending = SET;
    
	dma_transfer_number_config(DMA1,DMA_CH7, len);	//重新配置传输数据量    
    dma_memory_address_config(DMA1,DMA_CH7,DMA_MEMORY_0, bufAddr);  //配置数据区地址
    dma_channel_enable(DMA1, DMA_CH7);              //使能DMA并发送    
    return SUCCESS;
}

/************************************************************
*	函数名称：	Debug_GetDMASendingFlag
*	函数功能：	通讯用串口0的发送状态
*	入口参数：	
*	返回参数：	发送状态：SET；否则：RESET
*	说明：		
************************************************************/
FlagStatus Debug_GetDMASendingFlag(void)
{
    return g_Usart0DmaSending;
}

/************************************************************
*	函数名称：	Usart1_Init
*	函数功能：	通讯用串口1初始化（RS232）
*	入口参数：	baud - 待设定波特率
*	返回参数：	None
*	说明：		TX-PA2		RX-PA3
************************************************************/
void Usart1_Init(unsigned int baud)
{
    rcu_periph_clock_enable(RCU_GPIOA);//使能时钟
    rcu_periph_clock_enable(RCU_USART1);//使能串口1的时钟

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);//将功能复用到串口

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);//将功能复用到串口

    //PA2	TXD
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);//复用功能，上拉
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_2);//推挽输出，引脚频率25Mhz
    //PA3	RXD
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);//复用功能，上拉
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_3);//推挽输出，引脚频率25Mhz

    usart_deinit(USART1);

    usart_baudrate_set(USART1, baud);//设置波特率

    usart_parity_config(USART1,USART_PM_NONE);//奇偶校验

    usart_word_length_set(USART1, USART_WL_8BIT);//字长

    usart_stop_bit_set(USART1,USART_STB_1BIT);//停止位

    usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);	//关闭硬件流

    usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);	//关闭硬件流

    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);//发送模式使能

    usart_receive_config(USART1, USART_RECEIVE_ENABLE);//接收模式使能

    usart_enable(USART1);//使能串口
}

/************************************************************
*	函数名称：	Debug_SendStr
*
*	函数功能：	串口0字符串发送
*
*	入口参数：	pStr - 待发送数据
*
*	返回参数：	None
*
*	说明：
************************************************************/
void Debug_SendStr(char *pStr)
{
    while(*pStr != 0)
    {
        usart_data_transmit(USART0, *pStr++);		//发送数据
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));//等待发送完毕
    }	
}

/************************************************************
*	函数名称：	Debug_SendByte
*
*	函数功能：	串口0字节发送
*
*	入口参数：	ch - 待发送字符
*
*	返回参数：	None
*
*	说明：
************************************************************/
void Debug_SendByte(char ch)
{
    usart_data_transmit(USART0, ch);		//发送数据
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));//等待发送完毕
}	


/************************************************************
*	函数名称：	Debug_SendNByte
*
*	函数功能：	串口0字节组发送
*
*	入口参数：	pBuf - 待发送字符缓存
*				size - 字节长度
*
*	返回参数：	None
*
*	说明：
************************************************************/
void Debug_SendNByte(char *pBuf, uint16_t size)
{
	while(size--)
		Debug_SendByte(*pBuf++);		
}

/************************************************************
*	函数名称：	Debug_QueyChar
*
*	函数功能：	串口0查询是否接收到字符
*
*	入口参数：	pChar - 接收到的字符
*
*	返回参数：	接收到字符，返回SUCCESS，否则返回ERROR
*
*	说明：
************************************************************/
ErrStatus Debug_QueyChar(char *pChar)
{
    if(RESET != usart_flag_get(USART0, USART_FLAG_RBNE))
    {
        *pChar = (char) usart_data_receive(USART0);
        return SUCCESS;
    }
    return ERROR;
}

/************************************************************
*	函数名称：	Debug_WaitChar
*
*	函数功能：	串口0等待接收字符
*
*	入口参数：	None
*
*	返回参数：	接收到的字符
*
*	说明：
************************************************************/
char Debug_WaitChar(void)
{
    while(RESET == usart_flag_get(USART0, USART_FLAG_RBNE));
    return (char) usart_data_receive(USART0);   
}

/************************************************************
*	函数名称：	Debug_ClearChar
*
*	函数功能：	串口0清空接收缓冲
*
*	入口参数：	None
*
*	返回参数：	None
*
*	说明：
************************************************************/
void Debug_ClearChar(void)
{
    USART_DATA(USART0);		//读一次数据寄存器
}

/************************************************************
*	函数名称：	RS232_SendStr
*
*	函数功能：	串口1字符串发送
*
*	入口参数：	pStr - 待发送数据
*
*	返回参数：	None
*
*	说明：
************************************************************/
void RS232_SendStr(char *pStr)
{
    while(*pStr != 0)
    {
        usart_data_transmit(USART1, *pStr++);		//发送数据
        while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//等待发送完毕
    }	
}

/************************************************************
*	函数名称：	RS232_SendByte
*	函数功能：	串口1字节发送
*	入口参数：	ch - 待发送字符
*	返回参数：	None
*	说明：
************************************************************/
void RS232_SendByte(char ch)
{
    usart_data_transmit(USART1, ch);		//发送数据
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//等待发送完毕
}	

/************************************************************
*	函数名称：	RS232_SendBuf
*	函数功能：	串口1缓冲发送
*	入口参数：	pBuf - 待发送字符缓冲
*				size - 发送字符数量
*	返回参数：	None
*	说明：
************************************************************/
void RS232_SendBuf(char *pBuf, uint16_t size)
{
	while(size--)
	{
		usart_data_transmit(USART1, *pBuf++);		//发送数据
		while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//等待发送完毕
	}
}

/************************************************************
*	函数名称：	RS232_QueyChar
*	函数功能：	串口1查询是否接收到字符
*	入口参数：	pChar - 接收到的字符
*	返回参数：	接收到字符，返回SUCCESS，否则返回ERROR
*	说明：
************************************************************/
ErrStatus RS232_QueyChar(char *pChar)
{
    if(RESET != usart_flag_get(USART1, USART_FLAG_RBNE))
    {
        *pChar = (char) usart_data_receive(USART1);
        return SUCCESS;
    }
    return ERROR;
}

/************************************************************
*	函数名称：	RS232_WaitChar
*	函数功能：	串口1等待接收字符
*	入口参数：	None
*	返回参数：	接收到的字符
*	说明：
************************************************************/
char RS232_WaitChar(void)
{
    while(RESET == usart_flag_get(USART1, USART_FLAG_RBNE));
    return (char) usart_data_receive(USART1);   
}

/************************************************************
*	函数名称：	RS232_ClearChar
*	函数功能：	串口1清空接收缓冲
*	入口参数：	None
*	返回参数：	None
*	说明：
************************************************************/
void RS232_ClearChar(void)
{
    USART_DATA(USART1);		//读一次数据寄存器
}

/************************************************************
*	函数名称：	DMA1_Channel7_IRQHandler
*	函数功能：	调试串口DMA发送完成中断
*	入口参数：	None
*	返回参数：	None
*	说明：
************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
    if(SET == dma_interrupt_flag_get(DMA1,DMA_CH7,DMA_INT_FLAG_FTF))    //发生传输完成中断
    {
        dma_interrupt_flag_clear(DMA1,DMA_CH7,DMA_INT_FLAG_FTF);	    //清除传输完成中断 
        g_Usart0DmaSending = RESET; //清除发送标志
    }
}
/** ---	End of File ---------------------------------------------------------------------------------  **/ 

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
**							������ֱ��ʹ�ò�ѯģʽ����ͨѶ���ȽϷ�ʱ�������ڲ�����˵�ǿ��õġ�
**							��Ӧ�ó����У�Ӧʹ��DMA���з��ͣ������ͨѶ��ʵʱ��Ҫ�󲻸ߣ�Ҳ��ʹ��DMA���н���
**							����ͨѶʵʱ��Ҫ��ϸߣ��Խ���Ӧ����ʹ���жϷ�ʽ��������
**							оƬ��USART���ͨѶ���ʿɴ�9Mbps����ʹ�á�1MBps������ʱ������ʹ��DMA���н��ա�
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

static FlagStatus g_Usart0DmaSending = RESET;   //USART0��DMA����״̬�����ڷ���״̬ʱ�����ɽ�����һ�ַ���

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	Usart0_Init
*	�������ܣ�	�����ô���0��ʼ����DEBUG��
*	��ڲ�����	baud - ���趨������
*	���ز�����	None
*	˵����		TX-PA9		RX-PA10
************************************************************/
void Usart0_Init(unsigned int baud)
{
    rcu_periph_clock_enable(RCU_GPIOA);//ʹ��ʱ��
    rcu_periph_clock_enable(RCU_USART0);//ʹ�ܴ���0��ʱ��

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    //PA9	TXD
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_9);//�������������Ƶ��25Mhz
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);//���ù��ܣ�����

    //PA10	RXD
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_10);//�������������Ƶ��25Mhz
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);//���ù��ܣ�����

    usart_deinit(USART0);

    usart_baudrate_set(USART0, baud);//���ò�����

    usart_parity_config(USART0,USART_PM_NONE);//��żУ��

    usart_word_length_set(USART0, USART_WL_8BIT);//�ֳ�

    usart_stop_bit_set(USART0,USART_STB_1BIT);//ֹͣλ

    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);

    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);

    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);//����ģʽʹ��

    usart_receive_config(USART0, USART_RECEIVE_ENABLE);//����ģʽʹ��

    usart_enable(USART0);//ʹ�ܴ���
}

/************************************************************
*	�������ƣ�	Usart0_DMA_Init
*	�������ܣ�	�����ô���0 DMA��ʼ����DEBUG��: ����DMA��ʼ��
*	��ڲ�����	
*	���ز�����	None
*	˵����		DMA1(PERI4)��CH5 - USART0_RX(Debug);  CH7 - USART0_TX(Debug);
*               ר����3��Uint32���ݣ�ֻ��������һ��Uint16���ݣ�����Ϊ6
************************************************************/
void Usart0_DMA_Init(void)
{
	dma_multi_data_parameter_struct dma_init_struct;//��ʼ��ʹ�ýṹ��

    rcu_periph_clock_enable(RCU_DMA1);//ʹ��DMA1��ʱ��
	
	//���� USART0_TX DMA
    dma_deinit(DMA1, DMA_CH7);	//��λDMAͨ��CH4�Ĵ���
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;				//�洢��������ģʽ
	dma_init_struct.critical_value = DMA_FIFO_4_WORD;				//FIFO 4�ֿ�ʱ���洢����һ��
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;		//����ѭ������ģʽ
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;					//�����ȼ�������ADC�ɼ�DMA
	
	dma_init_struct.number = 0;				                        //���ݴ�������4*4=16�ı������ݲ���������
	
    dma_init_struct.memory0_addr = (uint32_t) 0; 		            //DMA���ͻ��壬�ݲ�ָ����ַ
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//�洢������ģʽ
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//�洢�����16bit
	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_4_BEAT;	//4��32bit����һ��ͻ������
	
    dma_init_struct.periph_addr = (uint32_t) &USART_DATA(USART0);	//DMA�����ַ
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//���������ģʽ
    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_8BIT;			//�������ݳ��ȣ�8λ
	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//����ͻ������	

    dma_multi_data_mode_init(DMA1, DMA_CH7, &dma_init_struct);		//�洢��ͻ����δ���	
    dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI4);	//DMA1����ͨ��ѡ�� USART0_TX 
    
    dma_interrupt_flag_clear(DMA1,DMA_CH7,DMA_INT_FLAG_FTF);	    //�����������ж�
    dma_interrupt_enable(DMA1,DMA_CH7,DMA_CHXCTL_FTFIE);            //ʹ��DMA��������ж�
    nvic_irq_enable(DMA1_Channel7_IRQn, 2, 2);                              //�ж����ȼ�2���ж������ȼ�2
    
    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);           //ʹ�ܴ��ڷ���DMA    
}

/************************************************************
*	�������ƣ�	Debug_DMASend
*	�������ܣ�	ͨѶ�ô���0ͨ��DMA����һ������
*	��ڲ�����	bufAddr - ���������ݻ����ַ
*               len     - ���������ݵ��ֽڳ��ȣ�����Ϊ16����������
*	���ز�����	���ͳɹ�������SUCCESS�����򷵻�ERROR
*	˵����		
************************************************************/
ErrStatus Debug_DMASend(uint32_t bufAddr, uint16_t len)
{
    if((len&0xF) || g_Usart0DmaSending==SET)         //����Ϊ16�����������ҵ�ǰ���������ݷ���
        return ERROR;
    
    g_Usart0DmaSending = SET;
    
	dma_transfer_number_config(DMA1,DMA_CH7, len);	//�������ô���������    
    dma_memory_address_config(DMA1,DMA_CH7,DMA_MEMORY_0, bufAddr);  //������������ַ
    dma_channel_enable(DMA1, DMA_CH7);              //ʹ��DMA������    
    return SUCCESS;
}

/************************************************************
*	�������ƣ�	Debug_GetDMASendingFlag
*	�������ܣ�	ͨѶ�ô���0�ķ���״̬
*	��ڲ�����	
*	���ز�����	����״̬��SET������RESET
*	˵����		
************************************************************/
FlagStatus Debug_GetDMASendingFlag(void)
{
    return g_Usart0DmaSending;
}

/************************************************************
*	�������ƣ�	Usart1_Init
*	�������ܣ�	ͨѶ�ô���1��ʼ����RS232��
*	��ڲ�����	baud - ���趨������
*	���ز�����	None
*	˵����		TX-PA2		RX-PA3
************************************************************/
void Usart1_Init(unsigned int baud)
{
    rcu_periph_clock_enable(RCU_GPIOA);//ʹ��ʱ��
    rcu_periph_clock_enable(RCU_USART1);//ʹ�ܴ���1��ʱ��

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);//�����ܸ��õ�����

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);//�����ܸ��õ�����

    //PA2	TXD
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);//���ù��ܣ�����
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_2);//�������������Ƶ��25Mhz
    //PA3	RXD
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);//���ù��ܣ�����
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_3);//�������������Ƶ��25Mhz

    usart_deinit(USART1);

    usart_baudrate_set(USART1, baud);//���ò�����

    usart_parity_config(USART1,USART_PM_NONE);//��żУ��

    usart_word_length_set(USART1, USART_WL_8BIT);//�ֳ�

    usart_stop_bit_set(USART1,USART_STB_1BIT);//ֹͣλ

    usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);	//�ر�Ӳ����

    usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);	//�ر�Ӳ����

    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);//����ģʽʹ��

    usart_receive_config(USART1, USART_RECEIVE_ENABLE);//����ģʽʹ��

    usart_enable(USART1);//ʹ�ܴ���
}

/************************************************************
*	�������ƣ�	Debug_SendStr
*
*	�������ܣ�	����0�ַ�������
*
*	��ڲ�����	pStr - ����������
*
*	���ز�����	None
*
*	˵����
************************************************************/
void Debug_SendStr(char *pStr)
{
    while(*pStr != 0)
    {
        usart_data_transmit(USART0, *pStr++);		//��������
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));//�ȴ��������
    }	
}

/************************************************************
*	�������ƣ�	Debug_SendByte
*
*	�������ܣ�	����0�ֽڷ���
*
*	��ڲ�����	ch - �������ַ�
*
*	���ز�����	None
*
*	˵����
************************************************************/
void Debug_SendByte(char ch)
{
    usart_data_transmit(USART0, ch);		//��������
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));//�ȴ��������
}	


/************************************************************
*	�������ƣ�	Debug_SendNByte
*
*	�������ܣ�	����0�ֽ��鷢��
*
*	��ڲ�����	pBuf - �������ַ�����
*				size - �ֽڳ���
*
*	���ز�����	None
*
*	˵����
************************************************************/
void Debug_SendNByte(char *pBuf, uint16_t size)
{
	while(size--)
		Debug_SendByte(*pBuf++);		
}

/************************************************************
*	�������ƣ�	Debug_QueyChar
*
*	�������ܣ�	����0��ѯ�Ƿ���յ��ַ�
*
*	��ڲ�����	pChar - ���յ����ַ�
*
*	���ز�����	���յ��ַ�������SUCCESS�����򷵻�ERROR
*
*	˵����
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
*	�������ƣ�	Debug_WaitChar
*
*	�������ܣ�	����0�ȴ������ַ�
*
*	��ڲ�����	None
*
*	���ز�����	���յ����ַ�
*
*	˵����
************************************************************/
char Debug_WaitChar(void)
{
    while(RESET == usart_flag_get(USART0, USART_FLAG_RBNE));
    return (char) usart_data_receive(USART0);   
}

/************************************************************
*	�������ƣ�	Debug_ClearChar
*
*	�������ܣ�	����0��ս��ջ���
*
*	��ڲ�����	None
*
*	���ز�����	None
*
*	˵����
************************************************************/
void Debug_ClearChar(void)
{
    USART_DATA(USART0);		//��һ�����ݼĴ���
}

/************************************************************
*	�������ƣ�	RS232_SendStr
*
*	�������ܣ�	����1�ַ�������
*
*	��ڲ�����	pStr - ����������
*
*	���ز�����	None
*
*	˵����
************************************************************/
void RS232_SendStr(char *pStr)
{
    while(*pStr != 0)
    {
        usart_data_transmit(USART1, *pStr++);		//��������
        while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//�ȴ��������
    }	
}

/************************************************************
*	�������ƣ�	RS232_SendByte
*	�������ܣ�	����1�ֽڷ���
*	��ڲ�����	ch - �������ַ�
*	���ز�����	None
*	˵����
************************************************************/
void RS232_SendByte(char ch)
{
    usart_data_transmit(USART1, ch);		//��������
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//�ȴ��������
}	

/************************************************************
*	�������ƣ�	RS232_SendBuf
*	�������ܣ�	����1���巢��
*	��ڲ�����	pBuf - �������ַ�����
*				size - �����ַ�����
*	���ز�����	None
*	˵����
************************************************************/
void RS232_SendBuf(char *pBuf, uint16_t size)
{
	while(size--)
	{
		usart_data_transmit(USART1, *pBuf++);		//��������
		while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));//�ȴ��������
	}
}

/************************************************************
*	�������ƣ�	RS232_QueyChar
*	�������ܣ�	����1��ѯ�Ƿ���յ��ַ�
*	��ڲ�����	pChar - ���յ����ַ�
*	���ز�����	���յ��ַ�������SUCCESS�����򷵻�ERROR
*	˵����
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
*	�������ƣ�	RS232_WaitChar
*	�������ܣ�	����1�ȴ������ַ�
*	��ڲ�����	None
*	���ز�����	���յ����ַ�
*	˵����
************************************************************/
char RS232_WaitChar(void)
{
    while(RESET == usart_flag_get(USART1, USART_FLAG_RBNE));
    return (char) usart_data_receive(USART1);   
}

/************************************************************
*	�������ƣ�	RS232_ClearChar
*	�������ܣ�	����1��ս��ջ���
*	��ڲ�����	None
*	���ز�����	None
*	˵����
************************************************************/
void RS232_ClearChar(void)
{
    USART_DATA(USART1);		//��һ�����ݼĴ���
}

/************************************************************
*	�������ƣ�	DMA1_Channel7_IRQHandler
*	�������ܣ�	���Դ���DMA��������ж�
*	��ڲ�����	None
*	���ز�����	None
*	˵����
************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
    if(SET == dma_interrupt_flag_get(DMA1,DMA_CH7,DMA_INT_FLAG_FTF))    //������������ж�
    {
        dma_interrupt_flag_clear(DMA1,DMA_CH7,DMA_INT_FLAG_FTF);	    //�����������ж� 
        g_Usart0DmaSending = RESET; //������ͱ�־
    }
}
/** ---	End of File ---------------------------------------------------------------------------------  **/ 

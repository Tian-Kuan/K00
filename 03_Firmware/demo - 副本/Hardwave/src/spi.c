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
** Descriptions:            SPI������8bit��Ϊ16bit֡
**							�������Զ�����ʱ�� ���߽��մ�ģʽ�����������Զ��ɼ�
**							�������Զ�����ģʽ �� �ֶ�����ģʽ���л�����
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
*	�������ƣ�	SPI0_Init
*	�������ܣ�	SPI0��ʼ��������FRAM����
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      (1):PA7 - MOSI
*	            (2):PA6 - MISO
*	            (3):PA5 - SCK
*	            (4):PA4 - NSS��	���NSSģʽ����ʹ��
*					������Ӳ��NSSģʽ��ΪNSS�����NSSDRV=1ʱ��Ϊ������ģʽ
*                   �����ԣ�ֻҪʹ��SPI��NSS��Ӳ��ģʽ���������Ϊ�͵�ƽ��
************************************************************/
void SPI0_Init(void)
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
	
    //����PA5/PA6/PA7ΪSPI��SCK/MISO/MOSI
    gpio_af_set(GPIOA, GPIO_AF_5, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
    //����PA4λIO����������������NSS
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);//���ģʽ��������
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);//�������������Ƶ��50Mhz
    gpio_bit_set(GPIOA, GPIO_PIN_4);    //�ߵ�ƽ���
     
	spi_i2s_deinit(SPI0);		//��λSPI0����
	
	 /* SPI0���� */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    spi_init_struct.device_mode          = SPI_MASTER;				//����ģʽ
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;  //ʱ�ӵ͵�ƽ����һ����
    spi_init_struct.nss                  = SPI_NSS_SOFT;			//ʹ��������ƣ��ɷ�ֹ�ж�Ӱ��
    spi_init_struct.prescale             = SPI_PSC_8;               //��Ƶ��Ϊ��10.5MHz
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
	
	spi_crc_polynomial_set(SPI0, 0x07);	//CRC����ʽ: CRC8 ITU ROHC
//	spi_crc_on(SPI0);		//ʹ��CRC
	
	spi_enable(SPI0);		//ʹ��SPI0	
}

#define SPI1_MFD_SRB_MSK	(~(SPI_CTL0_BDEN | SPI_CTL0_SWNSSEN | SPI_CTL0_MSTMOD))
#define SPI1_MODE_SRB		(SPI_CTL0_BDEN)				//����˫��ģʽ����ģʽ
#define SPI1_MODE_MFD		(SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN)	//˫�ߵ���ģʽ����ģʽ, ���Ƭѡ
/************************************************************
*	�������ƣ�	SPI1_Init
*	�������ܣ�	SPI1��ʼ��������ADC����
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      (1):PB15 - MOSI
*	            (2):PB14 - MISO
*	            (3):PB13 - SCK
*	            (4):PB12 - NSS��	���NSSģʽ����ʹ��
*					������Ӳ��NSSģʽ��ΪNSS�����NSSDRV=1ʱ��Ϊ������ģʽ
*                   �����ԣ�ֻҪʹ��SPI��NSS��Ӳ��ģʽ���������Ϊ�͵�ƽ��
*				ADS131M02��SPI����ʱ�����ϸ�Ҫ�����SPIҲ���ɹ�����TIģʽ��
*				����NSS�����Զ���������Ҫ������ƣ�DMA��Ϊ���ӣ���Ҫ���Ч�ʣ��ɿ���Timer����NSS��CLK�Ĵӻ�˫��ģʽ��SRBģʽ
*				��ǰ����£�Ҫʹ��DMA�����Կ���˫�жϷ�ʽ���� DRDY�жϣ�ʹ��Ƭѡ������SPI���ݶ�ȡ���� DMA��ȡ�����ݣ����жϣ����ж���ֹͣƬѡ
*				Ҳ���Կ������ж��н�����ʱ������ʹ��Ƭѡ����������Ҫ��֤Ƭѡ�ź�20ns���ϵĸߵ�ƽ��������ȡ�÷�ʽ��
*				WNOTE����Ƭѡ��ʱ�䱣�ֵ͵�ƽ��ADS131M��DRDY�����ᷢ����ֱ��SPI��ʱ��λ������ʹ��CRC�ɽ�������⣬����Ҫ���ԡ�
*
*				ȫ˫������ģʽMFD��
*					MSTMOD = 1, ����ģʽ		RO = 0��ȫ˫��ģʽ
*					BDEN = 0, ˫�ߵ���ģʽ		BDOEN = 0��˫��ʱ��Ч
*					NSSDRV = 0, ����NSS��		SWNSSEN = 1���������
*					SWNSS = 1������������
*				˫�������Ӵӻ�����ģʽSRB��
*					MSTMOD = 0, �ӻ�ģʽ		RO = 0������ʱ��Ч
*					BDEN = 1, ����˫��ģʽ		BDOEN = 0, ֻ�����ڽ���ģʽ
*					NSSDRV = 0, ����NSS��		SWNSSEN = 0��Ӳ������
*					SWNSS = 1��������
************************************************************/
void SPI1_Init(void)
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI1);
	
    //����PB13/PB14/PB15ΪSPI��SCK/MISO/MOSI
    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	gpio_bit_reset(GPIOB, GPIO_PIN_15);	//������MOSIΪGPIO�����ʱ�򣬱�֤���������Ϊ�͵�ƽ
	
    //����PB12ΪIO����������������NSS
	gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12);             			//SPI CS��ѡ����Ĭ��ΪSPI����,��ǰδ��Ч
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//���ģʽ��������
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);//�������������Ƶ��50Mhz
    gpio_bit_set(GPIOB, GPIO_PIN_12);    //�ߵ�ƽ���
        
	spi_i2s_deinit(SPI1);		//��λSPI1����
	
	 /* SPI1���� */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    spi_init_struct.device_mode          = SPI_MASTER;				//����ģʽ
    spi_init_struct.frame_size           = SPI_FRAMESIZE_16BIT;		//����SPI�����ݴ�С:SPI���ͽ���16λ֡�ṹ
	//spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;  //ʱ�ӵ͵�ƽ���ڶ����ؽ��գ� SPI MODE 1
    spi_init_struct.nss                  = SPI_NSS_SOFT;			//ʹ��������ƣ��ɷ�ֹ�ж�Ӱ��
    spi_init_struct.prescale             = SPI_PSC_4;               //��Ƶ��Ϊ��10.5MHz
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);
	spi_nss_output_enable(SPI1);		//ʹ��NSS��������ģʽ�£���λ��Ч
	
	//�������CRC����DMAʱ�����Զ�����CRC���ݺͽ���CRCУ��
	//����������ʱ�����������һ���ֺ����ֶ�����
//#ifdef ENABLE_CRC_IN	
//	spi_crc_polynomial_set(SPI1, 0x1021);	//CRC����ʽ: CCITT CRC
//	spi_crc_on(SPI1);		//ʹ��CRC
//#endif

	spi_enable(SPI1);		//ʹ��SPI1	
}

/************************************************************
*	�������ƣ�	EXADC_SpiMaster_Start
*	�������ܣ�	�ⲿADC��������SPIģʽ���ڲ�SPI�����ڴӻ�ģʽ
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      
************************************************************/
void EXADC_SpiMaster_Start(void)
{
	spi_disable(SPI1);	//����SPI���Խ�������	
	
	//���� ˫�������Ӵӻ�����ģʽSRB
	SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & SPI1_MFD_SRB_MSK) | SPI1_MODE_SRB;
	spi_nss_output_disable(SPI1);	
	
	//��SPI��\CS��ΪSPI���ܣ���MOSI��ΪGPIO���
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);	//SPI CS��Ϊ��ѡ��Ĭ��SPI���� 
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);//SPI MOSI��ΪGPIO��� 
	
	spi_enable(SPI1);	//��������SPI1
}

/************************************************************
*	�������ƣ�	EXADC_SpiMaster_Stop
*	�������ܣ�	�ⲿADC�����ڴ�SPIģʽ���ڲ�SPI����������ģʽ
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      
************************************************************/
void EXADC_SpiMaster_Stop(void)
{
	spi_disable(SPI1);	//����SPI���Խ�������	
	
	//��SPI��/CS��ΪGPIO�����MOSI��ΪSPI����
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_12);	//SPI /CS��ΪGPIO���������
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);		//SPI MOSI��Ϊ��ѡ��Ĭ��SPI���� 
	
	//����ȫ˫������ģʽMFD
	SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & SPI1_MFD_SRB_MSK) | SPI1_MODE_MFD;
	spi_nss_output_enable(SPI1);
	
	spi_enable(SPI1);	//��������SPI1
}


/************************************************************
*	�������ƣ�	SPI1_DMA_Init
*	�������ܣ�	SPI1 DMA��ʼ��������ADC���ݵ��շ�
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      DMA0��CH3 - SPI1_RX
*					  CH4 - SPI1_TX
*				SPI��DMAģʽ��ʱ����Ҫ��Ƭѡ����Ϊ�Զ�ģʽ
************************************************************/
void SPI1_DMA_Init(void)
{
	dma_multi_data_parameter_struct dma_init_struct;//��ʼ��ʹ�ýṹ��

    rcu_periph_clock_enable(RCU_DMA0);//ʹ��DMA��ʱ��
	
	//���� SPI1_RX DMA
    dma_deinit(DMA0, DMA_CH3);//��λDMAͨ��CH3�Ĵ���
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;				//���赽�洢��ģʽ
	dma_init_struct.critical_value = DMA_FIFO_2_WORD;				//FIFO�ﵽ2���֣�8�ֽڣ�ʱд��MEM
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;		//ѭ������ģʽ������PingPong����
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;				//�������ȼ�
	
	dma_init_struct.number = 0;				    					//���ݴ�����������16bit���ݲ���������
	
    dma_init_struct.memory0_addr = (uint32_t) g_SPI1DMARxBufPi; 	//DMA �洢��0��ַ
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//�洢������ģʽ
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//�洢�����32bit
	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_SINGLE;	//���δ��䣬FIFO��ֵ��������2�����δ���
	
    dma_init_struct.periph_addr = (uint32_t) &SPI_DATA(SPI1);		//DMA�����ַ
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//���������ģʽ
    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_16BIT;			//�������ݳ��ȣ�16λ
	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//���δ���	

    dma_multi_data_mode_init(DMA0, DMA_CH3, &dma_init_struct);		//�洢��ͻ����δ���	
    dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI0);	//DMA����ͨ��ѡ��SPI1_RX
	dma_switch_buffer_mode_enable(DMA0,DMA_CH3,ENABLE);				//ʹ�ܴ洢���л�ģʽ

	//���� SPI1_TX DMA
//    dma_deinit(DMA0, DMA_CH4);	//��λDMAͨ��CH4�Ĵ���
//    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;				//�洢��������ģʽ
//	dma_init_struct.critical_value = DMA_FIFO_1_WORD;				//����ģʽ��1�ִ���
//	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;		//����ѭ������ģʽ
//    dma_init_struct.priority = DMA_PRIORITY_HIGH;					//�����ȼ������ڽ���DMA
//	
//	dma_init_struct.number = 0;				    //���ݴ���������������16bit����*2���ݲ���������
//	
//    dma_init_struct.memory0_addr = (uint32_t) g_SPI1DMATxBuf; 		//DMA���ͻ���
//    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		//�洢������ģʽ
//	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;			//�洢�����32bit
//	dma_init_struct.memory_burst_width  = DMA_MEMORY_BURST_SINGLE;	//1���ֽ���һ��ͻ������
//	
//    dma_init_struct.periph_addr = (uint32_t) &SPI_DATA(SPI1);		//DMA�����ַ
//    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;		//���������ģʽ
//    dma_init_struct.periph_width = DMA_PERIPH_WIDTH_16BIT;			//�������ݳ��ȣ�16λ
//	dma_init_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;	//����ͻ������	

//    dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);		//�洢��ͻ����δ���	
//    dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);	//DMA����ͨ��ѡ��SPI1_TX
}


//��������Ҫ����ʱ
/************************************************************
*	�������ƣ�	SPI1_DMAReadOnce
*	�������ܣ�	ʹ��DMA������ȡSPI1һ������
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      ����3���ֵ�����
************************************************************/
//void SPI1_DMAReadOnce(void)
//{
//	dma_interrupt_flag_clear(DMA0,DMA_CH4,DMA_INT_FLAG_FTF);		//��Ҫ�����������жϣ�����
//	dma_transfer_number_config(DMA0,DMA_CH4,SPI1_DMAONE_SIZE*2);	//���÷�������	
//	dma_channel_enable(DMA0, DMA_CH4);	//ʹ��SPI1 DMAͨ���ķ���
//}

//���ڲ���TIģʽ������ADS131M��SPIʱ����ϸ�Ҫ��TIģʽ������
//void SPI1_TIEnable(uint8_t isTI)
//{	
//	spi_disable(SPI1);	//����SPI���Խ�������	
//	
//	if(isTI)
//	{
//		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12);	//����PB12ΪƬѡ
//		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
//		spi_ti_mode_enable(SPI1);
//	}
//	else
//	{
//		gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//���ģʽ��������
//		gpio_bit_set(GPIOB, GPIO_PIN_12);    //�ߵ�ƽ���
//		spi_ti_mode_disable(SPI1);
//	}

//	spi_enable(SPI1);	//��������SPI1	
//}

/************************************************************
*	�������ƣ�	SPI1_DMAStart
*	�������ܣ�	ʹ��DMA
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      
*   ע�⣺
*   			WNOTE: ��CPU��NSS����ΪӲ��ģʽ����ôNSS��SPIʹ��ʱ��һֱΪ��
************************************************************/
void SPI1_DMAStart(void)
{	
	//����DMA
	dma_interrupt_flag_clear(DMA0,DMA_CH3,DMA_INT_FLAG_FTF);								//��Ҫ�����������жϣ�����
	dma_transfer_number_config(DMA0,DMA_CH3, SPI1_DMABUF_SIZE);								//�������ô���������, ÿ�δ���16bit
	dma_switch_buffer_mode_config(DMA0,DMA_CH3,(uint32_t) g_SPI1DMARxBufPo,DMA_MEMORY_0);	//ѡ��ռ�0Ϊ��ʼMEM�洢��

	spi_i2s_data_receive(SPI1);	//����SPI1�Ľ���BUF;
	
	dma_channel_enable(DMA0, DMA_CH3);	 //ʹ��SPI1 DMAͨ���Ľ���
	spi_dma_enable(SPI1,SPI_DMA_RECEIVE);//ʹ��SPI1����DMA
	//spi_dma_enable(SPI1,SPI_DMA_TRANSMIT);//ʹ��SPI1����DMA
	
	EXADC_SpiMaster_Start();	//����SPI����
	EXADC_Timer_Start();		//����Timer���ò�����
}

/************************************************************
*	�������ƣ�	SPI1_DMAStop
*	�������ܣ�	����DMA
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      
************************************************************/
void SPI1_DMAStop(void)
{
	EXADC_Timer_Stop();		//��ԭTimer���ò�ֹͣ
	EXADC_SpiMaster_Stop();	//��ԭSPI����
	
//	spi_dma_disable(SPI1,SPI_DMA_TRANSMIT);	//����SPI1����DMA
	spi_dma_disable(SPI1,SPI_DMA_RECEIVE);	//����SPI1����DMA
	dma_channel_disable(DMA0, DMA_CH3);		//����SPI1 DMAͨ���Ľ��գ������³�ʼ���������
	
	//��CS�޸�ΪGPIO����ģʽ
//	spi_disable(SPI1);	//����SPI���Խ�������
//	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);//���ģʽ��������
//    gpio_bit_set(GPIOB, GPIO_PIN_12);    //�ߵ�ƽ���
//	SPI_CTL0(SPI1) |= SPI_NSS_SOFT;	//��NSS��Ϊ���ģʽ
//	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//��ͨͨѶ����8bitģʽ
//	spi_enable(SPI1);	//��������SPI1
}

/************************************************************
*	�������ƣ�	SPI1_GetDMABufAddr
*	�������ܣ�	��ȡDMA�����ַ
*	��ڲ�����	mem		- �ռ���
*	���ز�����	�ռ��ַ
*   ˵����      
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
*	�������ƣ�	FRAM_Transmit
*	�������ܣ�	ͨ��SPI0����һ������
*	��ڲ�����	pData   - �����͵����ݿռ�
*               size    - �����͵���������
*               timeout - ��ʱ������
*	���ز�����	����ʱ������ERROR�����򷵻�SUCCESS
*   ˵����      
************************************************************/
ErrStatus FRAM_Transmit( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
    while(size--)
    {
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //�ȴ����ͼĴ���Ϊ��
        {
            if(0 == iTimeout--) //��ʱ
                return ERROR;
        }
        spi_i2s_data_transmit(SPI0, *pData++);
    }
    
    //�ȴ����ͽ���
    iTimeout = timeout;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //�ȴ����ͼĴ���Ϊ��
    {
        if(0 == iTimeout--) //��ʱ
            return ERROR;
    }
    iTimeout = timeout;
    while(RESET != spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)) //�ȴ������շ����
    {
        if(0 == iTimeout--) //��ʱ
            return ERROR;
    }
    
    return SUCCESS;
}

/************************************************************
*	�������ƣ�	FRAM_TransmitCrc
*	�������ܣ�	ͨ��SPI0����һ���CRCУ�������
*	��ڲ�����	pData   - �����͵����ݿռ䣨��CRC��
*               size    - �����͵�������������CRC��
*               timeout - ��ʱ������
*	���ز�����	����ʱ������ERROR�����򷵻�SUCCESS
*   ˵����      
************************************************************/
ErrStatus FRAM_TransmitCrc( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
	ErrStatus iStatus = SUCCESS;
	
	spi_disable(SPI0);
	spi_crc_on(SPI0);	//ֻ����SPI����ʱ���޸�CRCʹ��״̬
	spi_enable(SPI0);
	
    while(--size)
    {
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //�ȴ����ͼĴ���Ϊ��
        {
            if(0 == iTimeout--) //��ʱ
			{
				iStatus = ERROR;
				goto ERROR;
			}
        }
        spi_i2s_data_transmit(SPI0, *pData++);
		if(size==1)	//���һ�����ݣ�׼������CRC
			spi_crc_next(SPI0);
    }
    
    //�ȴ����ͽ���
    iTimeout = timeout;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) //�ȴ����ͼĴ���Ϊ��
    {
        if(0 == iTimeout--) //��ʱ
		{
			iStatus = ERROR;
			goto ERROR;
		}
    }
	
    iTimeout = timeout;
    while(RESET != spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)) //�ȴ������շ���ϣ�CRC�ѷ���
    {
        if(0 == iTimeout--) //��ʱ
		{
			iStatus = ERROR;
			goto ERROR;
		}
    }
	
	*pData = spi_crc_get(SPI0,SPI_CRC_TX);	//��CRCУ���������
    spi_crc_error_clear(SPI0);				//�������У�����
	ERROR:
	spi_disable(SPI0);
	spi_crc_off(SPI0);
	spi_enable(SPI0);
    return iStatus;
}

/************************************************************
*	�������ƣ�	FRAM_Receive
*	�������ܣ�	ͨ��SPI0����һ������
*	��ڲ�����	pData   - �����͵����ݿռ�
*               size    - �����͵���������
*               timeout - ��ʱ������
*	���ز�����	����ʱ������ERROR�����򷵻�SUCCESS
*   ˵����      
************************************************************/
ErrStatus FRAM_Receive( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
    
    *pData = (uint8_t) spi_i2s_data_receive(SPI0);        //���ս��ռĴ���
    while(size--)
    {
        spi_i2s_data_transmit(SPI0, 0);   //����һ�η���
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))  //�ȴ����յ�����
        {
            if(0 == iTimeout--) //��ʱ
                return ERROR;
        }
        *pData++ = (uint8_t) spi_i2s_data_receive(SPI0);
    }
    return SUCCESS;
}

/************************************************************
*	�������ƣ�	FRAM_ReceiveCrc
*	�������ܣ�	ͨ��SPI0����һ�����ݣ�������CRC8У��
*	��ڲ�����	pData   - �����յ����ݿռ䣨��CRC�ֽڣ�
*               size    - �����յ�������������CRC�ֽڣ�
*               timeout - ��ʱ������
*	���ز�����	����ʱ������ERROR�����򷵻�SUCCESS
*   ˵����      
************************************************************/
ErrStatus FRAM_ReceiveCrc( uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t iTimeout;
	ErrStatus iStatus = SUCCESS;
    
    *pData = (uint8_t) spi_i2s_data_receive(SPI0);        //���ս��ռĴ���

	spi_disable(SPI0);
	spi_crc_on(SPI0);	//ֻ����SPI����ʱ���޸�CRCʹ��״̬
	spi_enable(SPI0);
	
    while(--size)
    {
        spi_i2s_data_transmit(SPI0, 0);   //����һ�η���
		if(size == 1)	//���һ�����ݣ�׼������CRCУ���ֽ�
			spi_crc_next(SPI0);
        iTimeout = timeout;
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))  //�ȴ����յ�����
        {
            if(0 == iTimeout--) //��ʱ
			{
                iStatus = ERROR;
				goto ERROR;
			}
        }
        *pData++ = (uint8_t) spi_i2s_data_receive(SPI0);
    }
	//���ղ�У��CRC
	iTimeout = timeout;
	while(SET == spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS))
	{
		if(0 == iTimeout--)
		{
			iStatus = ERROR;
			goto ERROR;
		}
	}
	
	if(SET == spi_i2s_flag_get(SPI0, SPI_FLAG_CRCERR))//CRCУ�����
	{
		spi_crc_error_clear(SPI0);
		iStatus = ERROR;
	}
	else
		*pData = spi_crc_get(SPI0,SPI_CRC_RX);	//���һ��ΪCRCУ��
	
	ERROR:
	spi_disable(SPI0);
	spi_crc_off(SPI0);
	spi_enable(SPI0);
    return iStatus;
}


/** ---	End of File ---------------------------------------------------------------------------------  **/ 

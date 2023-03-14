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
**							ADC FRAME Set As 32bit��No CRC(for the 32bit CRC align left, CPU Can't auto calculate) 
**							if CRC is needed, Set ADC Frame As 16bit
**							When DMA��SPI frame is 16bit width��Data is 32bit width
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
** Descriptions:            ��ADS131M�ӿ�����֡���Ϊ�̶�16bit֡���Լ��ٴ�������ʹ�����������
**							ֻ֧��2ͨ����16bit����
**							�޸�ADC DRDY��RESET��������
**							��SPI����ʱ��CPU������SPI��ģʽ����ʹ���������ţ�
**								PB12  - SPI_CS(δ�ã���ΪGPIO IN��
**								PC9   - CS(GPIO OUT)
**								PB13  - SPI_SCK, OUT
**								PD14  - SCK(δ�ã���ΪGPIO IN)
**								PB14  - SPI_MISO, IN
**								PB15  - SPI_MOSI, OUT
**							��SPI�ɼ����ݣ��Զ�DMAʱ��CPU������SPI��ģʽ����ʹ���������ţ�
**								PB12  - SPI_CS�� IN
**								PC9   - CS OUT, Timer2_CH3
**								PB13  - SPI_SCK, IN
**								PD14  - SCK OUT, Timer3_CH2
**								PB14  - SPI_MISO, IN
**								PB15  - MOSI(δ��)
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
//static uint16_t	g_ADSRegMap[NUM_REGISTERS];	//ADS131M�Ĵ���ӳ��

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
//void ADS131M_DefaultReg(void);
//uint16_t ADS131M_EnforceMode(uint16_t data);
ErrStatus ADS131M_TxRxArray(uint16_t *pSrc, uint16_t *pDst, uint16_t size);
ErrStatus SPI1_WaitFlag(uint32_t flag, FlagStatus status);
ErrStatus ADS131M_WriteRegF24(uint8_t addr, uint16_t data);//�ڸ�λ��Ӧʹ�øú�����ADC�Ĵ�����дΪ16bit֡��

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/** ---------------    ADS131M�������    --------------- **/
/************************************************************
*	�������ƣ�	ADS131M_Config
*	�������ܣ�	ADS131Mʹ��Ĭ������
*	��ڲ�����	None
*	���ز�����	ERROR: ����SUCCESS���ɹ���
*	˵����		WNOTE: ���ʹ��CRC����������CRCУ���룬�ᵼ��������������������⣻�´���ӦΪStatus
************************************************************/
ErrStatus ADS131M_Config(void)
{
	//��ADS131M���г�ʼ������	
	if( SUCCESS != ADS131M_WriteRegF24(CLOCK_ADDRESS, 	//����24λ֡ģʽ����ʱ�ӼĴ���
					CLOCK_CHALL_DISABLED |			//�������вɼ�ͨ���Ա���ʹ�ö�֡��������
					CLOCK_OSR_2048 |				//����2048������������2kHz����Ƶ��
					CLOCK_PWR_HR))					//ʹ�ø߾��Ȳɼ�ģʽ
		return ERROR;
	
	if( SUCCESS != ADS131M_WriteRegF24(MODE_ADDRESS,		//����24λ֡ģʽ����ģʽ�Ĵ���
					(MODE_DEFAULT & (~(MODE_WLENGTH_MASK | MODE_DRDY_FMT_MASK | MODE_RESET_MASK))) | MODE_WLENGTH_16BIT | MODE_DRDY_FMT_NEG_PULSE_FIXED_WIDTH)) //��Ĭ�����ø�Ϊ16bit֡����DRDY����ģʽ
		return ERROR;
	
	return SUCCESS;
	
	//  ���Ŵ󣬲���Ĭ������
	//  ADS131M_WriteReg(GAIN1_ADDRESS,		//��������Ĵ���
	//	GAIN1_PGAGAIN0_1 |					//����ο���ѹΪ1.2V������Ϊ��1.2V����˲���Ҫ�Ŵ�
	//	GAIN1_PGAGAIN1_1);				
	
	//  ������ֵ�Ĵ��������üĴ���
	//	������ն������ֵ��⣬������

	//  �ݲ������ɼ�ʱ��
	//ADS131M_WriteReg(CLOCK_ADDRESS, 	//����ʱ�ӼĴ���
	//	CLOCK_CH0_EN_ENABLED |			
	//	CLOCK_CH1_EN_ENABLED |
	//	CLOCK_OSR_2048 |				//����2048������������2kHz����Ƶ��
	//	CLOCK_PWR_HR);					//ʹ�ø߾��Ȳɼ�ģʽ
}


/************************************************************
*	�������ƣ�	ADS131M_Init
*	�������ܣ�	��ADS131M���г�ʼ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����		ADC ��������
*				PC6	- DRDY IN	���Ѿ���Timer2_Init������ΪGPIO���룩
*				PD15 - RESET OUT
*				CH0 - I�źţ�Sin���ͺ�����90�㣨Y�ᣩ
*				CH1 - Q�źţ�Cos��������ͬ�ࣨX�ᣩ
************************************************************/
void ADS131M_Init(void)
{
	Timer7_Init();		//��ʼ��ADS131Mʱ�ӣ���ʱ��Ϊ8.4MHz��Ҫ�õ�8.192MHz������ʹ��I2Sʱ�ӡ�	
	Timer2_Init();		//������Timer�Զ��ɼ�ģʽ����\CS�ź�
	Timer3_Init();		//������Timer�Զ��ɼ�ģʽ����SCLK�ź�
	//I2SCLK_Init();	//���ڲ���I2Sʱ�ӣ�ʹ��ADCʱ��Ϊ8.192MHz
	SPI1_DMA_Init();	//��ʼ��SPI��DMA
	SPI1_Init();		//��ʼ��SPI
	
	rcu_periph_clock_enable(RCU_SYSCFG);	//�����ж�

	//��ʼ��GPIO: DRDY/RESET
	rcu_periph_clock_enable(RCU_GPIOD);//����GPIOD��ʱ��
    //gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_8);		//PD8 DRDY IN
    //gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);//����Ϊ����ģʽ������
    gpio_mode_set(ADC_RESET_BANK, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ADC_RESET_PIN);			//PD15 Reset, ���, ����	
    gpio_output_options_set(ADC_RESET_BANK, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ADC_RESET_PIN);//�������������Ƶ��50Mhz
    gpio_bit_reset(ADC_RESET_BANK, ADC_RESET_PIN);//��λ��Ч
	
	delay_1ms(50);	//�ȴ�50ms���ϵ��ȶ�ʱ��
	ADS131M_Reset();//��ADC���и�λ
}

/************************************************************
*	�������ƣ�	ADS131M_Reset
*	�������ܣ�	��ADS131M����Ӳ����λ
*	��ڲ�����	None
*	���ز�����	����λ�ɹ����򷵻�SUCCESS�����򷵻�ERROR
*	˵����		
************************************************************/
ErrStatus ADS131M_Reset(void)
{
	uint32_t iTimeOut = 0x0280DE80;	//��Լ����
	uint16_t rxData;
	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//��λǰ����SPIΪ8֡ģʽ�Ա��⸴λ����ʴ���
	
	gpio_bit_reset(ADC_RESET_BANK, ADC_RESET_PIN);	//�͵�ƽ��λ		
	delay_1ms(2);	//��λ����250us���˴�Ϊ1ms	
	gpio_bit_set(ADC_RESET_BANK, ADC_RESET_PIN);//�ߵ�ƽ�������λ���	
	//delay_1ms(2);	//�ȴ���λ���
	
	//��λ����ҪDRDY��Ϊ�ߵ�ƽ��������ͨѶ
	while(iTimeOut--)
	{
		if(RESET != gpio_input_bit_get(ADC_DRDY_BANK, ADC_DRDY_PIN))
			break;
	}
	
    if(ERROR == ADS131M_SendCmd(OPCODE_NULL, &rxData))	//��λ�󣬶��ص�һ������ӦΪID
        return ERROR;
    
    //�����Ӧ�Ƿ�ΪID
    if(rxData != 0xFF22)    //����ӦΪ0x0011˵���յ�֡�����ǲ�����
        return ERROR;
	
	return ADS131M_Config();		//���г�ʼ������	
}

/************************************************************
*	�������ƣ�	ADS131M_Reset
*	�������ܣ�	��ADS131M����ͬ�������¿�ʼ�ɼ���������ڲ�FIFO����
*	��ڲ�����	bWaitDrdy:  �ȴ�DRDY��ɸߵ�ƽ, 0Ϊ���ȴ�������Ϊ�ȴ�
*	���ز�����	None
*	˵����		ͬ��ʱ��Ӧ���������жϣ��������ӳٶ���λ
*				����ʾ�����۲죬ʵ��ͬ������ԼΪ 2.5us��Լ420��ʱ�����ڣ�
*				RESET/SYNC: 1~2047��ADCʱ������Ϊͬ����2048��������Ϊ��λ
************************************************************/
void ADS131M_Sync(uint8_t bWaitDrdy)
{
	int i;
	uint32_t iTimeOut = 0x0280DE80;	//��Լ����
	if(bWaitDrdy)	//��ʹ�ñ��ش���ʱ�����ⲿ����irq���ڵ�ƽ����ʱ�����ڲ�����irq;
		__disable_irq();		//�����ж�����ͬ��ʱ���������λ
	gpio_bit_reset(GPIOD, GPIO_PIN_15);	//�͵�ƽͬ��		
	//����1~2047��ʱ��(Լ122ns~249us)��Ϊͬ��ADC�����¿�ʼ�ɼ�
	for(i=0;i<100;i++);	//Լ2.5us��ÿ��ѭ��Լ4��ʱ�����ڣ�
	gpio_bit_set(GPIOD, GPIO_PIN_15);//�ߵ�ƽ�����ͬ�����
	
	//�ڵ�ƽ������ʱ����Ҫ�ȴ�DRDY��ߵ�ƽ
	//�ڱ��ش�����ʱ�򣬲���Ҫ�ȴ�DRDY��ߵ�ƽ
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
*	�������ƣ�	ADS131M_ReadData
*	�������ܣ�	��ȡ�ɼ���������
*	��ڲ�����	pDataSt - ���ݴ洢��
*	���ز�����	����ERROR����ȷ��SUCCESS
*	˵����		��������Ϊ16bit֡
*               �ɼ����ݾ�������
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
*	�������ƣ�	ADS131M_ReadReg
*	�������ܣ�	��ȡ�Ĵ�������
*	��ڲ�����	addr	- �Ĵ�����ַ
*				pRxData - ���ջ�����
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		
************************************************************/
ErrStatus ADS131M_ReadReg(uint8_t addr, uint16_t *pRxData)
{
	uint16_t opcode;
	
	addr &= 0x3F;
	opcode = OPCODE_RREG | (((uint16_t) addr)<<7);
	
	if(ERROR == ADS131M_SendCmd(opcode, pRxData))	//��������
		return ERROR;
	
	if(ERROR == ADS131M_SendCmd(OPCODE_NULL, pRxData))	//��������
		return ERROR;
	
	//g_ADSRegMap[addr] = *pRxData;	
	return SUCCESS;
}

/************************************************************
*	�������ƣ�	ADS131M_WriteReg
*	�������ܣ�	����16bit֡��ʽ��д��Ĵ�������
*	��ڲ�����	addr	- �Ĵ�����ַ
*				data	- ��д������
*				bCheck  - ��д������ݽ��ж���У��
*						  ADCSPI_CHECK_YES 	  - (1)��Ҫ����У��
*						  ADCSPI_CHECK_NONE   - (0)�����ж���У��
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		
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
	
	if(bCheck == ADCSPI_CHECK_YES)	//��Ҫ����У��
	{
		if(ERROR == ADS131M_ReadReg(addr, rxBuf))	//�ض����ݽ���ȷ�ϣ�����bcheckҪ���֡����
			return ERROR;
	
		if(rxBuf[0] != data)
			return ERROR;	
	}

	return SUCCESS;
}

/************************************************************
*	�������ƣ�	ADS131M_WriteRegF24
*	�������ܣ�	����24bit֡��ʽд��Ĵ�������
*	��ڲ�����	addr	- �Ĵ�����ַ
*				data	- ��д������
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		ֻ��ϵͳ����ʱʹ��24bit֡д�룬������Ϊ16bit֡
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
*	�������ƣ�	ADS131M_SoftReset
*	�������ܣ�	����λ������������λ
*	��ڲ�����	None
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		
************************************************************/
ErrStatus ADS131M_SoftReset(void)
{
    uint16_t rxBuf[CHANNEL_COUNT+1+1];  //ͨ����+CRC+OPCODE
    uint16_t txBuf[CHANNEL_COUNT+1+1] = {0};    
    
    txBuf[0] = OPCODE_RESET;
    
	//spi_i2s_data_frame_format_config(SPI1,SPI_FRAMESIZE_8BIT);	//��λǰ����SPIΪ8֡ģʽ�Ա��⸴λ����ʴ���
	
	if(ERROR == ADS131M_TxRxArray(txBuf, rxBuf, CHANNEL_COUNT+1+1))  //����16bit���գ����͸�λָ��֡
        return ERROR;
    
    delay_1ms(1);   //�ȴ���λ���
    
    if(ERROR == ADS131M_SendCmd(OPCODE_NULL, rxBuf))	//��λ�󣬶��ص�һ������ӦΪID
        return ERROR;
    
    //�����Ӧ�Ƿ�ΪID
    if(rxBuf[0] != 0xFF22)    //����ӦΪ0x0011˵���յ�֡�����ǲ�����
        return ERROR;
    
	return ADS131M_Config();		//���г�ʼ������	
}

/************************************************************
*	�������ƣ�	ADS131M_LockReg
*	�������ܣ�	�����Ĵ������ݣ���ֹ���޸ģ��Խ���רע�����ݲɼ�
*	��ڲ�����	None
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		����16bit����֡
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
*	�������ƣ�	ADS131M_UnlockReg
*	�������ܣ�	�����Ĵ���������ԼĴ��������޸�
*	��ڲ�����	None
*	���ز�����	����ERROR���ɹ���SUCCESS
*	˵����		ֻ����16bits֡��
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
*	�������ƣ�	SPI1_WaitFlag
*	�������ܣ�	�ȴ�SPI1��ĳ��״̬����
*	��ڲ�����	flag   - �ȴ����¼�
*				status - �¼���״̬��SET/RESET)
*	���ز�����	����ʱ��ERROR,���򷵻�SUCCESS
*   ˵����      
************************************************************/
ErrStatus SPI1_WaitFlag(uint32_t flag, FlagStatus status)
{
	uint32_t iTimeout = ADS131M_TIMEOUT_VALUE;
	while(status != spi_i2s_flag_get(SPI1, flag))  //�ȴ����յ�����
	{
		if(0 == iTimeout--) //��ʱ
			return ERROR;
	}
	return SUCCESS;
}

/************************************************************
*	�������ƣ�	ADS131M_SendCmd
*	�������ܣ�	��ADS131M����һ������
*	��ڲ�����	opcode	- �����͵������룬ֻ��Ϊ�������
*						NULL	�޲���
*						STANDBY	����
*						WAKEUP	����*		
*				pRxData  - ���յ�16bit����
*	���ز�����	���󣬷���ERROR������SUCCESS
*	˵����		��������Ӧʹ��ר�ú���:
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
*	�������ƣ�	ADS131M_TxRxArray
*	�������ܣ�	ͨ��SPI1�շ�һ������
*	��ڲ�����	pSrc    - �����͵����ݿռ�
*				pDst	- �����յ����ݿռ�
*				size	- �����յ����ݸ���
*	���ز�����	����ʱ������ERROR,�ɹ�SUCCESS
*   ˵����      Ĭ�ϰ���16bit֡������   
************************************************************/
ErrStatus ADS131M_TxRxArray(uint16_t *pSrc, uint16_t *pDst, uint16_t size)
{
	uint16_t i;
	
    *pDst = spi_i2s_data_receive(SPI1);        //���ս��ռĴ���
	
	ADS131M_CSLow();	//ʹ��Ƭѡ
	
	for(i=0; i<size; i++)
	{
		spi_i2s_data_transmit(SPI1, pSrc[i]);   //����һ�η���//	
		if(ERROR == SPI1_WaitFlag(SPI_FLAG_RBNE, SET))
		{
			ADS131M_CSHigh();	//Ƭѡ����
			return ERROR;
		}
		*pDst++ = spi_i2s_data_receive(SPI1);	
	}
	
	ADS131M_CSHigh();	//Ƭѡ����
    return SUCCESS;	
}


/** ---------------    ADS131M�ɼ����    --------------- **/
/************************************************************
*	�������ƣ�	ADS131M_StartSample
*	�������ܣ�	����ADC�Ĳɼ�
*	��ڲ�����	
*	���ز�����	None
*   ˵����      �ɽ���DMA�ɼ�������ѯ��ʽ�ɼ�
************************************************************/
void ADS131M_StartSample(void)
{
	ADS131M_WriteReg(CLOCK_ADDRESS, 	//����ʱ�ӼĴ�����ʹ�ܲɼ�ͨ��
		CLOCK_CH0_EN_ENABLED |			
		CLOCK_CH1_EN_ENABLED |
		CLOCK_OSR_2048 |				//����2048������������2kHz����Ƶ��
		CLOCK_PWR_HR,					//ʹ�ø߾��Ȳɼ�ģʽ	
		ADCSPI_CHECK_NONE);
	ADS131M_LockReg();					//�����Ĵ������������޸�	
	
	//Ӳ��ͬ��
	ADS131M_Sync(1);					//���½���ͬ�������ҵȴ�DRDY��ߵ�ƽ
}

/************************************************************
*	�������ƣ�	ADS131M_StopSample
*	�������ܣ�	ֹͣADC�Ĳɼ�
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      
************************************************************/
void ADS131M_StopSample(void)
{
	ADS131M_UnlockReg();				//�����Ĵ�������
	ADS131M_WriteReg(CLOCK_ADDRESS, 	//����ʱ�ӼĴ���
		CLOCK_CHALL_DISABLED |			//��ֹͨ���ɼ�
		CLOCK_OSR_2048 |				//����2048������������2kHz����Ƶ��
		CLOCK_PWR_HR,					//ʹ�ø߾��Ȳɼ�ģʽ		
		ADCSPI_CHECK_NONE);
}

/************************************************************
*	�������ƣ�	ADS131M_StartDMA
*	�������ܣ�	����ADC��DMA�ɼ�
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      ���ѯDMA����״̬�Ի�ȡ����
************************************************************/
void ADS131M_StartDMA(void)
{
	__disable_irq();		//��ֹ���ж϶����µ�һ��DMA��ʧ
	ADS131M_StartSample();
	SPI1_DMAStart();
	
	ADS131M_Sync(0);		//ʹ���½��ش���������ȴ�
	EXADC_TimerDrdyEnable();
	__enable_irq();
}

/************************************************************
*	�������ƣ�	ADS131M_StartDMA
*	�������ܣ�	����ADC��DMA�ɼ�
*	��ڲ�����	None
*	���ز�����	None
*   ˵����      ���ѯDMA����״̬�Ի�ȡ����
************************************************************/
void ADS131M_StopDMA(void)
{
	SPI1_DMAStop();
	ADS131M_StopSample();
}

/************************************************************
*	�������ƣ�	ADS131M_QuerySmpNum
*	�������ܣ�	���ò�ѯ�ɼ���ʽ�ɼ�ָ������������
*	��ڲ�����	pBuf	- ���ݻ���
*				size	- �ɼ�������CH0/CH1��������
*	���ز�����	None
*	˵����		���CH0���ڵͰ��ַ����CH1���ڸ߰��ַ��
*				���ڵ�һ���ɼ��������ϴ󣬻ᶪ����һ��
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
	iInc = 0;	//���������ڶ�����һ��
	
	ADS131M_StartSample();

	while(iCnt<=size)	//��һ��ᱻ�������ʶ�ȡһ��
	{
		if(RESET == gpio_input_bit_get(ADC_DRDY_BANK, ADC_DRDY_PIN))	//���ݵ���
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

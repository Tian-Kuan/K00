/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               IIC.c
** Latest modified Date:    2022-6-3
** Latest Version:          1.0
** Descriptions:            Used for IIC1 Access
**							MCP4561, Support 100kHz/400kHz/3.4MHz(Use 400kHz)
**								EEPROM��NV Wiper, 0~256 Step,WiperLockAA
**                          PCLK1: AHB/4 = 42MHz; 
**                          PB10 - SCL
**                          PB11 - SDA
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-25
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
#include "iic.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
#define I2C_TIMEOUT_LONG		(0xA000)
//#define I2C_TIMEOUT_SHORT       (0x1000)

/** ---	Start of Para define 	---------------------------------------------------------------------  **/


/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
/***********************************************************
*	�������ƣ�	I2C1_Init
*	�������ܣ�	IIC1Ӳ����ʼ��
*	��ڲ�����	��
*	���ز�����	��
*	˵����		PB10 - SCL
*               PB11 - SDA
***********************************************************/
void I2C1_Init(void)
{	
    rcu_periph_clock_enable(RCU_GPIOB);	//ʹ��GPIOB����ʱ��
    rcu_periph_clock_enable(RCU_I2C1);	//ʹ��IIC����ʱ��
	
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_10);	//���Ź��ܸ��õ�IIC
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_11);	//���Ź��ܸ��õ�IIC
	
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ,GPIO_PIN_10);	//ʹ�����2MHz
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ,GPIO_PIN_11);	//ʹ�����2MHz
	
	//IICʱ������: MCP4561��400kHzҪ��ߵ�ƽ>0.6us, �͵�ƽ>1.3us; ʹ��1:2ʱ����0.83us����1.67us������Ҫ��
	//��ʹ��100kHz����Ҫ��ߵ�ƽ>4us���͵�ƽ>4.7us��ʵ��ʹ��1��1ģʽ���ߵ͸�5us��Ҳ����Ҫ��
    i2c_clock_config(I2C1, 100000,I2C_DTCY_2);	//ʹ��100kHz��׼ģʽ
  
    i2c_mode_addr_config(I2C1,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,0x55);  //IICģʽ��ʱ������, CPU IIC��ַ��Ϊ0x55

	i2c_analog_noise_filter_enable(I2C1);//����ģ�������˲�����ȡ��
	
	i2c_ackpos_config(I2C1,I2C_ACKPOS_NEXT);	//ʹ����һ���ݷ���ACK�źţ��ɷ�ֹ���жϷ�������

    i2c_ack_config(I2C1,I2C_ACK_ENABLE);	//ʹ��Ӧ��	

    i2c_enable(I2C1);	//ʹ��IIC
}

/***********************************************************
*	�������ƣ�	I2C1_WaitEvent
*	�������ܣ�	�ȴ�I2Cָ���¼�����
*	��ڲ�����	event	- �ȴ����¼�
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		
***********************************************************/
ErrStatus I2C1_WaitEvent(uint32_t event)
{
    uint32_t overtime = I2C_TIMEOUT_LONG;
    uint32_t status;
	while(overtime--)
	{
        status = I2C_STAT0(I2C1) | (I2C_STAT1(I2C1)<<16);
		if( (status & event) == event)  //Ҫ����¼�����
			return SUCCESS;
	}
	return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_WaitFlag
*	�������ܣ�	�ȴ�I2Cָ����־״̬����
*	��ڲ�����	flag	- �ȴ��ı�־
*               status  - ��Ҫ�ı�־״̬
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		
***********************************************************/
ErrStatus I2C1_WaitFlag(i2c_flag_enum flag, FlagStatus status)
{
    uint32_t overtime = I2C_TIMEOUT_LONG;
	while(overtime--)
	{
		if( i2c_flag_get(I2C1,flag) == status)  //Ҫ��ı�־����
			return SUCCESS;
	}
	return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_WriteByte
*	�������ܣ�	��I2C����д��һ���Ĵ�����ַ��һ���ֽڵ�����
*	��ڲ�����	devAddr	- �豸��ַ
*               regAddr - �Ĵ�����ַ���׸�����
*               data    - ��д�������
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		
***********************************************************/
ErrStatus I2C1_WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
 	if(ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//�ȴ����߿���
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//��ʼ�źţ����ͺ󣬱��뷢��ֹͣ�źŲ��ܽ���
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto WriteByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//�ӻ���ַ		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto WriteByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//���ͼĴ�����ʼ��ַ���׸�����
    //BTC/TBE//TR/MASTER/I2CBSY:�ֽڷ��ͽ���/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ��Ĵ�����ַ������ɣ����յ���Ӧ
        goto WriteByteError;	 
    
    i2c_data_transmit(I2C1, data);	//��������
    if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ����ݷ�����ɣ����յ���Ӧ
    {
        i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
        return SUCCESS;
    }
        
    WriteByteError:
    i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
    return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_WriteNByte
*	�������ܣ�	��I2C����д��һ���Ĵ�����ַ�Ͷ���ֽڵ�����
*	��ڲ�����	devAddr	- �豸��ַ
*               pData   - ��д������ݻ���(���׸���ַ�����ݣ�
*               size    - ��д�����������
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		���Ĵ�����ַ��Ϊ���ݷ�����pData��
***********************************************************/
ErrStatus I2C1_WriteNByte(uint8_t devAddr, uint8_t *pData, uint8_t size)
{   
 	if(size==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//�ȴ����߿���
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//��ʼ�źţ����ͺ󣬱��뷢��ֹͣ�źŲ��ܽ���
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto WriteNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//�ӻ���ַ		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto WriteNByteError;	        
    
    while(size--)
    {
        i2c_data_transmit(I2C1, *pData++);	                            //��������
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ����ݷ�����ɣ����յ���Ӧ
            goto WriteNByteError;
    }
    i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
    return SUCCESS;
    
    WriteNByteError:
    i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
    return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_WriteMultiple
*	�������ܣ�	��I2C���߽�ͬһ���ݶ��д��һ���Ĵ�����ַ
*	��ڲ�����	devAddr	- �豸��ַ
*               data    - ��д�������
*               times   - ��д��Ĵ���
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		���Ĵ�����ַ��Ϊ���ݷ�����pData��
***********************************************************/
ErrStatus I2C1_WriteMultiple(uint8_t devAddr, uint8_t data, uint8_t times)
{   
 	if(times==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//�ȴ����߿���
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//��ʼ�źţ����ͺ󣬱��뷢��ֹͣ�źŲ��ܽ���
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto WriteNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//�ӻ���ַ		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto WriteNByteError;	        
    
    while(times--)
    {
        i2c_data_transmit(I2C1, data);	                                //��������
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ����ݷ�����ɣ����յ���Ӧ
            goto WriteNByteError;
    }
    i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
    return SUCCESS;
    
    WriteNByteError:
    i2c_stop_on_bus(I2C1);		//����ֹͣ�ź�
    return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_ReadByte
*	�������ܣ�	��I2C��ȡһ���ֽ�
*	��ڲ�����	devAddr	- �豸��ַ
*               regAddr - �Ĵ�����ַ
*               pData   - ��ȡ���ݻ���
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		�����������£�
*               1. �ȴ����߿��У�Ȼ��ʹ�ܵ�ǰӦ�����
*               2. ������ʼ�źţ����ȴ��������
*               3. ���ʹӻ���ַд�����ȴ���Ӧ
*               4. ���ͼĴ�����ַ�����ȴ���Ӧ
*               5. �ٴη�����ʼ�źţ����ȴ��������
*               6. ���ʹӻ���ַ����ͬʱ����Ӧ���ٵȴ���Ӧ
*               7. ׼������ֹͣ�źţ��ȴ����ݵ��������
*	ע�⣺		
*				�ö�ȡһ�ֽڷ�ʽ���ʺ�MCP4561, MCP4561������Ҫ��ȡ2bytes
***********************************************************/
ErrStatus I2C1_ReadByte(uint8_t devAddr, uint8_t regAddr, uint8_t *pData)
{ 
 	if(ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//�ȴ����߿���
		return ERROR; 
    
    i2c_ackpos_config(I2C1,I2C_ACKPOS_CURRENT);         //����Ϊ���Ƶ�ǰ�����ֽ��Ƿ���Ӧ��
    i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //����Ӧ��
    
    i2c_start_on_bus(I2C1);		//��ʼ�źţ����ͺ󣬱��뷢��ֹͣ�źŲ��ܽ���
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto ReadByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//�ӻ���ַ		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto ReadByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//���ͼĴ�����ʼ��ַ���׸�����
    //BTC/TBE//TR/MASTER/I2CBSY:�ֽڷ��ͽ���/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ��Ĵ�����ַ������ɣ����յ���Ӧ
        goto ReadByteError;

    i2c_start_on_bus(I2C1);		//�ٴ���������
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto ReadByteError;
    
    i2c_master_addressing(I2C1, devAddr, I2C_RECEIVER);	//�ӻ���ַ������ģʽ	
	//i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //����Ӧ��
    //ADDSEND//MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ//����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto ReadByteError;
    
    i2c_stop_on_bus(I2C1);		                    //׼������ֹͣ�ź�
    
    //RBNE//MASTER/I2CBSY: ���ռĴ����ǿ�//����ģʽ/����ͨѶ
    if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//�ȴ����ݽ������
    {
        *pData = i2c_data_receive(I2C1);
        return SUCCESS;
    }
    else
        return ERROR;
    
    ReadByteError:
    i2c_stop_on_bus(I2C1);		                    //׼������ֹͣ�ź�
    return ERROR;
}

/***********************************************************
*	�������ƣ�	I2C1_ReadNByte
*	�������ܣ�	��I2C��ȡһ���ֽ�
*	��ڲ�����	devAddr	- �豸��ַ
*               regAddr - �Ĵ�����ַ
*               pData   - ��ȡ���ݻ���
*               size    - ��ȡ���ݵ�����
*	���ز�����	�����ʱ������ERROR������SUCCESS
*	˵����		�����������£�
*               1. �ȴ����߿��У�Ȼ��ʹ�ܵ�ǰӦ����ƣ�1�ֽڣ����Ƶ�ǰ������������һ�����ֽڣ�
*               2. ������ʼ�źţ����ȴ��������
*               3. ���ʹӻ���ַд�����ȴ���Ӧ
*               4. ���ͼĴ�����ַ�����ȴ���Ӧ
*               5. �ٴη�����ʼ�źţ����ȴ��������
*               6. ���ʹӻ���ַ������1�ֽ�ʱ����Ӧ�𣩣��ٵȴ���Ӧ
*               һ�ֽ�ʱ:
*               1-7. ׼������ֹͣ�ź�
*               1-8. �ȴ����ݵ��������
*               ���ֽ�ʱ��
*               2-7. ����Ӧ�𣬵ȴ�����ͣ�٣�����2�ֽڣ�
*               2-8. ����ֹͣ�źţ���ȡ2������
*               ���ڵ���3�ֽ�ʱ��
*               3-7. �ȴ������յ�N-3�ֽ�
*               3-8. �ȴ�����ͣ�٣�����2�ֽڣ�
*               3-9. ����Ӧ�𣬽��յ�N-2�ֽڣ�����ֹͣ�źţ����յ�N-1�ֽ�
*               3-10. �ȴ�������ݵ�����յ�N�ֽ�
***********************************************************/
ErrStatus I2C1_ReadNByte(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size)
{  
 	if(size==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//�ȴ����߿���
		return ERROR;     
    
    if(size>1)
    {
        i2c_ackpos_config(I2C1,I2C_ACKPOS_NEXT);           //����Ϊ������һ�����ֽ��Ƿ���Ӧ��  
        i2c_ack_config(I2C1,I2C_ACK_ENABLE);               //ʹ��Ӧ��
    }
    else    //ֻ��һ���ֽڵ�ʱ���������ַ��Ӧǰ�����ò�����Ӧ��
    {
        i2c_ackpos_config(I2C1,I2C_ACKPOS_CURRENT);        //����Ϊ���Ƶ�ǰ�����ֽ��Ƿ���Ӧ��
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //����Ӧ��
    }
    
    i2c_start_on_bus(I2C1);		//��ʼ�źţ����ͺ󣬱��뷢��ֹͣ�źŲ��ܽ���
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto ReadNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//�ӻ���ַ		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto ReadNByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//���ͼĴ�����ʼ��ַ���׸�����
    //BTC/TBE//TR/MASTER/I2CBSY:�ֽڷ��ͽ���/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//�ȴ��Ĵ�����ַ������ɣ����յ���Ӧ
        goto ReadNByteError;

    i2c_start_on_bus(I2C1);		//�ٴ���������
    //SBSEND//MASTER/I2CBSY: ��ʼ�ź��ѷ���//��������ģʽ/I2C����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//�ȴ���ʼ�źŷ�����ɣ���������ģʽ
        goto ReadNByteError;
    
    i2c_master_addressing(I2C1, devAddr, I2C_RECEIVER);	//�ӻ���ַ������ģʽ	
    //if(1 == size)
    //    i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //����Ӧ��
    //ADDSEND//MASTER/I2CBSY: ��ַ���Ͳ��õ���Ӧ//����ģʽ/����ͨѶ
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))	//�ȴ�I2C��ַ������ɣ����յ���Ӧ
        goto ReadNByteError;
    
    while(size>3)
    {
         //RBNE//MASTER/I2CBSY: ���ռĴ����ǿ�//����ģʽ/����ͨѶ
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//�ȴ����ݽ������
           goto ReadNByteError;
        *pData++ = i2c_data_receive(I2C1);
        size--;
    }
    
    if(2==size)
    {
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);           //����Ӧ��[ʱ��ܹؼ�]��STM32Ҫ���ڴ˴����ã���GD���ֲ��У�Ҫ����ADDSEND���ǰ����
        //BTC//TR/MASTER/I2CBSY:2�ֽڽ��ս���/���ͼĴ�����//���Ͷ�/����ģʽ/����ͨѶ
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED_TWICE))	//�ȴ�����ͣ��״̬����ʱ�����������յ�
            goto ReadNByteError;
        i2c_stop_on_bus(I2C1);		                    //����ֹͣ�ź�
        *pData++ = i2c_data_receive(I2C1);
        *pData = i2c_data_receive(I2C1);
    }
    else if(1==size)
    {    
        i2c_stop_on_bus(I2C1);		                    //׼������ֹͣ�ź�        
        //RBNE//MASTER/I2CBSY: ���ռĴ����ǿ�//����ģʽ/����ͨѶ
        if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//�ȴ����ݽ������
            *pData = i2c_data_receive(I2C1);
        else
            return ERROR;
    }
    else    //size==3
    {
        //BTC//TR/MASTER/I2CBSY:2�ֽڽ��ս���//���Ͷ�/����ģʽ/����ͨѶ
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED_TWICE))	//�ȴ�����ͣ��״̬����ʱ�����������յ�
            goto ReadNByteError;
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);           //����Ӧ��
        *pData++ = i2c_data_receive(I2C1);              //����һ�����ݣ�������һ������
        i2c_stop_on_bus(I2C1);		                    //����ֹͣ�ź�
        *pData++ = i2c_data_receive(I2C1);
        
         //RBNE//MASTER/I2CBSY: ���ռĴ����ǿ�//����ģʽ/����ͨѶ
        if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//�ȴ�������ݽ������
            *pData = i2c_data_receive(I2C1);
        else
            return ERROR;
    }    
    return SUCCESS;
    
    ReadNByteError:
    i2c_stop_on_bus(I2C1);		                    //����ֹͣ�ź�
    return ERROR;
}


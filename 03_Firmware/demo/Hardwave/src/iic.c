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
**								EEPROM，NV Wiper, 0~256 Step,WiperLockAA
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
*	函数名称：	I2C1_Init
*	函数功能：	IIC1硬件初始化
*	入口参数：	无
*	返回参数：	无
*	说明：		PB10 - SCL
*               PB11 - SDA
***********************************************************/
void I2C1_Init(void)
{	
    rcu_periph_clock_enable(RCU_GPIOB);	//使能GPIOB引脚时钟
    rcu_periph_clock_enable(RCU_I2C1);	//使能IIC外设时钟
	
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_10);	//引脚功能复用到IIC
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_11);	//引脚功能复用到IIC
	
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ,GPIO_PIN_10);	//使用最高2MHz
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ,GPIO_PIN_11);	//使用最高2MHz
	
	//IIC时钟配置: MCP4561在400kHz要求高电平>0.6us, 低电平>1.3us; 使用1:2时，高0.83us，低1.67us，满足要求
	//若使用100kHz，则要求高电平>4us，低电平>4.7us；实际使用1：1模式，高低各5us，也满足要求
    i2c_clock_config(I2C1, 100000,I2C_DTCY_2);	//使用100kHz标准模式
  
    i2c_mode_addr_config(I2C1,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,0x55);  //IIC模式和时钟配置, CPU IIC地址设为0x55

	i2c_analog_noise_filter_enable(I2C1);//启用模拟噪声滤波，可取消
	
	i2c_ackpos_config(I2C1,I2C_ACKPOS_NEXT);	//使能下一数据发送ACK信号，可防止被中断发生错误

    i2c_ack_config(I2C1,I2C_ACK_ENABLE);	//使能应答	

    i2c_enable(I2C1);	//使能IIC
}

/***********************************************************
*	函数名称：	I2C1_WaitEvent
*	函数功能：	等待I2C指定事件出现
*	入口参数：	event	- 等待的事件
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		
***********************************************************/
ErrStatus I2C1_WaitEvent(uint32_t event)
{
    uint32_t overtime = I2C_TIMEOUT_LONG;
    uint32_t status;
	while(overtime--)
	{
        status = I2C_STAT0(I2C1) | (I2C_STAT1(I2C1)<<16);
		if( (status & event) == event)  //要求的事件发生
			return SUCCESS;
	}
	return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_WaitFlag
*	函数功能：	等待I2C指定标志状态出现
*	入口参数：	flag	- 等待的标志
*               status  - 需要的标志状态
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		
***********************************************************/
ErrStatus I2C1_WaitFlag(i2c_flag_enum flag, FlagStatus status)
{
    uint32_t overtime = I2C_TIMEOUT_LONG;
	while(overtime--)
	{
		if( i2c_flag_get(I2C1,flag) == status)  //要求的标志出现
			return SUCCESS;
	}
	return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_WriteByte
*	函数功能：	往I2C总线写入一个寄存器地址和一个字节的数据
*	入口参数：	devAddr	- 设备地址
*               regAddr - 寄存器地址或首个数据
*               data    - 待写入的数据
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		
***********************************************************/
ErrStatus I2C1_WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
 	if(ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//等待总线空闲
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//起始信号，发送后，必须发送停止信号才能结束
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto WriteByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//从机地址		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: 地址发送并得到相应/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto WriteByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//发送寄存器起始地址或首个数据
    //BTC/TBE//TR/MASTER/I2CBSY:字节发送结束/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待寄存器地址发送完成，并收到响应
        goto WriteByteError;	 
    
    i2c_data_transmit(I2C1, data);	//发送数据
    if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待数据发送完成，并收到响应
    {
        i2c_stop_on_bus(I2C1);		//发送停止信号
        return SUCCESS;
    }
        
    WriteByteError:
    i2c_stop_on_bus(I2C1);		//发送停止信号
    return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_WriteNByte
*	函数功能：	往I2C总线写入一个寄存器地址和多个字节的数据
*	入口参数：	devAddr	- 设备地址
*               pData   - 待写入的数据缓存(含首个地址或数据）
*               size    - 待写入的数据数量
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		将寄存器地址作为数据放在了pData中
***********************************************************/
ErrStatus I2C1_WriteNByte(uint8_t devAddr, uint8_t *pData, uint8_t size)
{   
 	if(size==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//等待总线空闲
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//起始信号，发送后，必须发送停止信号才能结束
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto WriteNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//从机地址		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: 地址发送并得到相应/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto WriteNByteError;	        
    
    while(size--)
    {
        i2c_data_transmit(I2C1, *pData++);	                            //发送数据
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待数据发送完成，并收到响应
            goto WriteNByteError;
    }
    i2c_stop_on_bus(I2C1);		//发送停止信号
    return SUCCESS;
    
    WriteNByteError:
    i2c_stop_on_bus(I2C1);		//发送停止信号
    return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_WriteMultiple
*	函数功能：	往I2C总线将同一数据多次写入一个寄存器地址
*	入口参数：	devAddr	- 设备地址
*               data    - 待写入的数据
*               times   - 待写入的次数
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		将寄存器地址作为数据放在了pData中
***********************************************************/
ErrStatus I2C1_WriteMultiple(uint8_t devAddr, uint8_t data, uint8_t times)
{   
 	if(times==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//等待总线空闲
		return ERROR;
    
    i2c_start_on_bus(I2C1);		//起始信号，发送后，必须发送停止信号才能结束
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto WriteNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//从机地址		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: 地址发送并得到相应/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto WriteNByteError;	        
    
    while(times--)
    {
        i2c_data_transmit(I2C1, data);	                                //发送数据
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待数据发送完成，并收到响应
            goto WriteNByteError;
    }
    i2c_stop_on_bus(I2C1);		//发送停止信号
    return SUCCESS;
    
    WriteNByteError:
    i2c_stop_on_bus(I2C1);		//发送停止信号
    return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_ReadByte
*	函数功能：	从I2C读取一个字节
*	入口参数：	devAddr	- 设备地址
*               regAddr - 寄存器地址
*               pData   - 读取数据缓存
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		操作步骤如下，
*               1. 等待总线空闲，然后使能当前应答控制
*               2. 发送起始信号，并等待发送完毕
*               3. 发送从机地址写，并等待响应
*               4. 发送寄存器地址，并等待响应
*               5. 再次发送起始信号，并等待发送完毕
*               6. 发送从机地址读，同时禁用应答，再等待响应
*               7. 准备发送停止信号，等待数据到达，并保存
*	注意：		
*				该读取一字节方式不适合MCP4561, MCP4561最少需要读取2bytes
***********************************************************/
ErrStatus I2C1_ReadByte(uint8_t devAddr, uint8_t regAddr, uint8_t *pData)
{ 
 	if(ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//等待总线空闲
		return ERROR; 
    
    i2c_ackpos_config(I2C1,I2C_ACKPOS_CURRENT);         //配置为控制当前接收字节是否发送应答
    i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //禁用应答
    
    i2c_start_on_bus(I2C1);		//起始信号，发送后，必须发送停止信号才能结束
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto ReadByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//从机地址		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: 地址发送并得到相应/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto ReadByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//发送寄存器起始地址或首个数据
    //BTC/TBE//TR/MASTER/I2CBSY:字节发送结束/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待寄存器地址发送完成，并收到响应
        goto ReadByteError;

    i2c_start_on_bus(I2C1);		//再次启动总线
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto ReadByteError;
    
    i2c_master_addressing(I2C1, devAddr, I2C_RECEIVER);	//从机地址，接收模式	
	//i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //禁用应答
    //ADDSEND//MASTER/I2CBSY: 地址发送并得到响应//主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto ReadByteError;
    
    i2c_stop_on_bus(I2C1);		                    //准备发送停止信号
    
    //RBNE//MASTER/I2CBSY: 接收寄存器非空//主机模式/正在通讯
    if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//等待数据接收完成
    {
        *pData = i2c_data_receive(I2C1);
        return SUCCESS;
    }
    else
        return ERROR;
    
    ReadByteError:
    i2c_stop_on_bus(I2C1);		                    //准备发送停止信号
    return ERROR;
}

/***********************************************************
*	函数名称：	I2C1_ReadNByte
*	函数功能：	从I2C读取一个字节
*	入口参数：	devAddr	- 设备地址
*               regAddr - 寄存器地址
*               pData   - 读取数据缓存
*               size    - 读取数据的数量
*	返回参数：	如果超时，返回ERROR，否则SUCCESS
*	说明：		操作步骤如下，
*               1. 等待总线空闲，然后使能当前应答控制（1字节，控制当前；其他控制下一接收字节）
*               2. 发送起始信号，并等待发送完毕
*               3. 发送从机地址写，并等待响应
*               4. 发送寄存器地址，并等待响应
*               5. 再次发送起始信号，并等待发送完毕
*               6. 发送从机地址读，（1字节时禁用应答），再等待响应
*               一字节时:
*               1-7. 准备发送停止信号
*               1-8. 等待数据到达，并保存
*               二字节时：
*               2-7. 禁用应答，等待接收停顿（接收2字节）
*               2-8. 发送停止信号，读取2个数据
*               大于等于3字节时：
*               3-7. 等待并接收第N-3字节
*               3-8. 等待接收停顿（接收2字节）
*               3-9. 禁用应答，接收第N-2字节；发送停止信号，接收第N-1字节
*               3-10. 等待最后数据到达，接收第N字节
***********************************************************/
ErrStatus I2C1_ReadNByte(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size)
{  
 	if(size==0 || ERROR == I2C1_WaitFlag(I2C_FLAG_I2CBSY, RESET))	//等待总线空闲
		return ERROR;     
    
    if(size>1)
    {
        i2c_ackpos_config(I2C1,I2C_ACKPOS_NEXT);           //配置为控制下一接收字节是否发送应答  
        i2c_ack_config(I2C1,I2C_ACK_ENABLE);               //使能应答
    }
    else    //只有一个字节的时候在清除地址响应前，设置不发送应答
    {
        i2c_ackpos_config(I2C1,I2C_ACKPOS_CURRENT);        //配置为控制当前接收字节是否发送应答
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //禁用应答
    }
    
    i2c_start_on_bus(I2C1);		//起始信号，发送后，必须发送停止信号才能结束
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto ReadNByteError;

    i2c_master_addressing(I2C1, devAddr, I2C_TRANSMITTER);	//从机地址		
    //ADDSEND/TBE//TR/MASTER/I2CBSY: 地址发送并得到相应/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto ReadNByteError;	        
    
    i2c_data_transmit(I2C1, regAddr);	//发送寄存器起始地址或首个数据
    //BTC/TBE//TR/MASTER/I2CBSY:字节发送结束/发送寄存器空//发送端/主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))	//等待寄存器地址发送完成，并收到响应
        goto ReadNByteError;

    i2c_start_on_bus(I2C1);		//再次启动总线
    //SBSEND//MASTER/I2CBSY: 开始信号已发送//进入主机模式/I2C正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))	//等待开始信号发送完成，进入主机模式
        goto ReadNByteError;
    
    i2c_master_addressing(I2C1, devAddr, I2C_RECEIVER);	//从机地址，接收模式	
    //if(1 == size)
    //    i2c_ack_config(I2C1,I2C_ACK_DISABLE);               //禁用应答
    //ADDSEND//MASTER/I2CBSY: 地址发送并得到响应//主机模式/正在通讯
    if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))	//等待I2C地址发送完成，并收到响应
        goto ReadNByteError;
    
    while(size>3)
    {
         //RBNE//MASTER/I2CBSY: 接收寄存器非空//主机模式/正在通讯
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//等待数据接收完成
           goto ReadNByteError;
        *pData++ = i2c_data_receive(I2C1);
        size--;
    }
    
    if(2==size)
    {
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);           //禁用应答，[时间很关键]；STM32要求在此处禁用；在GD的手册中，要求在ADDSEND清除前禁用
        //BTC//TR/MASTER/I2CBSY:2字节接收结束/发送寄存器空//发送端/主机模式/正在通讯
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED_TWICE))	//等待接收停顿状态，此时有两个数据收到
            goto ReadNByteError;
        i2c_stop_on_bus(I2C1);		                    //发送停止信号
        *pData++ = i2c_data_receive(I2C1);
        *pData = i2c_data_receive(I2C1);
    }
    else if(1==size)
    {    
        i2c_stop_on_bus(I2C1);		                    //准备发送停止信号        
        //RBNE//MASTER/I2CBSY: 接收寄存器非空//主机模式/正在通讯
        if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//等待数据接收完成
            *pData = i2c_data_receive(I2C1);
        else
            return ERROR;
    }
    else    //size==3
    {
        //BTC//TR/MASTER/I2CBSY:2字节接收结束//发送端/主机模式/正在通讯
        if(ERROR == I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED_TWICE))	//等待接收停顿状态，此时有两个数据收到
            goto ReadNByteError;
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);           //禁用应答
        *pData++ = i2c_data_receive(I2C1);              //读走一个数据，启动下一个接收
        i2c_stop_on_bus(I2C1);		                    //发送停止信号
        *pData++ = i2c_data_receive(I2C1);
        
         //RBNE//MASTER/I2CBSY: 接收寄存器非空//主机模式/正在通讯
        if(ERROR != I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))	//等待最后数据接收完成
            *pData = i2c_data_receive(I2C1);
        else
            return ERROR;
    }    
    return SUCCESS;
    
    ReadNByteError:
    i2c_stop_on_bus(I2C1);		                    //发送停止信号
    return ERROR;
}


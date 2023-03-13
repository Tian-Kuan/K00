/****************************************Explains********************************************************
**                            		Code for BuMan.
**                                      
**            https://blog.csdn.net/weixin_43490708?spm=1010.2135.3001.5343
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               hmc5883l.c
** Latest modified Date:    2023-3-13
** Latest Version:          1.0
** Descriptions:            Configure & test hmc5883l
**                          MCP4561: Electronic compass                                
**
**--------------------------------------------------------------------------------------------------------
** Created by:              BuMan
** Created date:            2023-3-13
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
//#include "gd32f4xx.h"
//#include "systick.h"
#include "i2c.h"
#include "hmc5883l.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/

/* device address */
#define HMC5883_ADDR			(0X1E)	//7-bit address			:	0001 1110
#define HMC5883_WRITE_ADDR		(0X3C)	//8-bit read address	:	0011 1100
#define HMC5883_READ_ADDR		(0X3D)	//8-bit read address	:	0011 1101


/* Register address */
#define HMC5883_REG_CONFIGA 	(0x00)	//配置寄存器A：用于配置该装置设置的数据输出速率和测量配置
#define HMC5883_REG_CONFIGB 	(0x01)	//配置寄存器B: 用于设置装置的增益
#define HMC5883_REG_MODE 		(0x02)	//用来设定装置的操作模式
#define HMC5883_REG_X_MSB 		(0x03)	//输出X寄存器A中存储测量结果中的MSB（高位数据）
#define HMC5883_REG_X_LSB 		(0x04)	//输出X寄存器B中存储测量结果中的MSB（低位数据）
#define HMC5883_REG_Z_MSB 		(0x05)	//输出Y寄存器A中存储测量结果中的MSB（高位数据）
#define HMC5883_REG_Z_LSB 		(0x06)	//输出Y寄存器B中存储测量结果中的MSB（低位数据）
#define HMC5883_REG_Y_MSB 		(0x07)	//输出Z寄存器A中存储测量结果中的MSB（高位数据） 
#define HMC5883_REG_Y_LSB 		(0x08)	//输出Z寄存器B中存储测量结果中的MSB（低位数据） 
#define HMC5883_REG_IDENTA		(0x09)	//识别寄存器A
#define HMC5883_REG_IDENTB		(0x0A)	//识别寄存器B
#define HMC5883_REG_IDENTC		(0x0B)	//识别寄存器C


/* Configuration Register A */
//CRA7:Bit CRA7 is reserved for future function. Set to 0 when configuring CRA.
//MA1 to MA0(CRA6 to CRA5):Select number of samples averaged (1 to 8) per measurement output.
#define HMC5883_REG_CRA_MS_NORAL     (0x00)	//default
#define HMC5883_REG_CRA_MS_XYZ_P     (0x01)
#define HMC5883_REG_CRA_MS_XYZ_N     (0x02)
#define HMC5883_REG_CRA_MS_REV       (0x03)
//MA4 to MA2(CRA4 to CRA2):Data Output Rate Bits. 
#define HMC5883_REG_CRA_DO_Hz_0_75   (0x00)
#define HMC5883_REG_CRA_DO_Hz_1_5    (0x04)
#define HMC5883_REG_CRA_DO_Hz_3      (0x08)
#define HMC5883_REG_CRA_DO_Hz_7_5    (0x0C)
#define HMC5883_REG_CRA_DO_Hz_15     (0x10) //default
#define HMC5883_REG_CRA_DO_Hz_30     (0x14)
#define HMC5883_REG_CRA_DO_Hz_75     (0x18)
#define HMC5883_REG_CRA_DO_Hz_NONE   (0x1C)
//MA1 to MA0(CRA1 to CRA0):Measurement Configuration Bits. 
#define HMC5883_REG_CRA_MA_AVG_1     (0x00)
#define HMC5883_REG_CRA_MA_AVG_2     (0x20)
#define HMC5883_REG_CRA_MA_AVG_4     (0x40)
#define HMC5883_REG_CRA_MA_AVG_8     (0x60) //default
	

/* Configuration Register B */
//GN2 to GN0(CRB7 to CRB5):Gain Configuration Bits. PS:(CRB4 to CRB0)These bits must be cleared for correct operation.
#define HMC5883_REG_GN_G1370    (0x00)
#define HMC5883_REG_GN_G1090    (0x20)	// default
#define HMC5883_REG_GN_G820     (0x40)
#define HMC5883_REG_GN_G660     (0x60)
#define HMC5883_REG_GN_G440     (0x80)
#define HMC5883_REG_GN_G390     (0xA0)
#define HMC5883_REG_GN_G330     (0xC0)
#define HMC5883_REG_GN_G230     (0xE0)


/* Mode Register */   
//MD1 to MD0(MR1 to MR0):Mode Select Bits.PS:(MR7 to MR2)Set this pin to enable High Speed I2C, 3400kHz.
#define HMC5883_REG_MODE_MR_CON      (0x00)    
#define HMC5883_REG_MODE_MR_SIG      (0x01) // default   
#define HMC5883_REG_MODE_MR_IDLE1    (0x02)    
#define HMC5883_REG_MODE_MR_IDLE2    (0x03) 









/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
#define MCP4561_I2C_ADDR        (0x5C)  //MCP45x1的设备地址：0101 11 A0 0 = 0x5c, A0引脚接地 所以A0 = 0 
#define MCP4561_STEP_MAX        (0x100) //最大步数

//Command Operation bits
#define MCP4561_CMD_MSK         (0x0C)
#define MCP4561_CMD_WRITE       (0x00)  //Write Data
#define MCP4561_CMD_READ        (0x0C)  //Read Data
#define MCP4561_CMD_INC         (0x04)  //Incremen, Only for volatile
#define MCP4561_CMD_DEC         (0x08)  //Decrement, Only for volatile

//MSB Data
#define MCP4561_MSBDATA_MSK     (0x03)

//Memory Address
#define MCP4561_MEM_MSK         (0xF0)
#define MCP4561_MEM_BIT         (4u)    //MEM Address bit position
#define MCP4561_MEM_SIZE        (0x10)  //MEM Size
#define MCP4561_MEM_START       (0x0)   //The Start Address of registers
#define MCP4561_MEM_VWIPER0     (0x0)   //Volatile Wiper0
#define MCP4561_MEM_VWIPER1     (0x1)   //Volatile Wiper1
#define MCP4561_MEM_NVWIPER0    (0x2)   //None Volatile Wiper0
#define MCP4561_MEM_NVWIPER1    (0x3)   //None Volatile Wiper1
#define MCP4561_MEM_VTCON       (0x4)   //Volatile TCON Register
#define MCP4561_MEM_STATUS      (0x5)   //Status Register
#define MCP4561_MEM_EEPROM_START (0x6)  //Data EEPROM Start Addr
#define MCP4561_MEM_EEPROM_END  (0xF)   //Data EEPROM End Addr

//Status Bit
#define BITMSK_STATUS_WP        (0x01)  //1: EEPROM写保护；0：无写保护
#define BITMSK_STATUS_WIL0      (0x02)  //1: 电位器0的Wiper和TCON写保护；0：无写保护
#define BITMSK_STATUS_WIL1      (0x04)  //1: 电位器1的Wiper和TCON写保护；0：无写保护
#define BITMSK_STATUS_EEWA      (0x08)  //1: 正在写入EEPROM；0：未写入数据

//TCON Bit: 上电默认0x1FF
#define BITMSK_TCON_GCEN        (0x100) //1: 使能接受通用呼叫地址(0000h); 0: 禁用
#define BITMSK_TCON_R1HW        (0x80)  //1: 电位器1没有强制为硬件Shutdown配置；0：强制为硬件Shutdown
#define BITMSK_TCON_R1A         (0x40)  //1: P1A接入电位器；0：断开
#define BITMSK_TCON_R1W         (0x20)  //1: P1W接入电位器；0：断开
#define BITMSK_TCON_R1B         (0x10)  //1: P1B接入电位器；0：断开
#define BITMSK_TCON_R0HW        (0x08)  //1: 电位器0没有强制为硬件Shutdown配置；0：强制为硬件Shutdown
#define BITMSK_TCON_R0A         (0x04)  //1: P0A接入电位器；0：断开
#define BITMSK_TCON_R0W         (0x02)  //1: P0W接入电位器；0：断开
#define BITMSK_TCON_R0B         (0x01)  //1: P0B接入电位器；0：断开

/** ---	Start of Para define 	---------------------------------------------------------------------  **/
void MCP4561_DisplayAll(void);
void MCP4561_ReviseRegs(void);
void MCP4561_ScanTest(void);

tdCS_TestList func_mcp4561_test[]=
{
	{MCP4561_DisplayAll,   "Display All Registers"         },
	{MCP4561_ReviseRegs,   "Modify specified Register"     },
	{MCP4561_ManualAdjust, "Compensate Amp Manual Adjust"  },
    {MCP4561_ScanTest,     "Test Scan the resistor"        },
	{0,0}
};

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
/** ---	芯片操作函数 -----------------------------------  **/
/************************************************************
*	函数名称：	MCP4561_Init
*	函数功能：	初始化电位器
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void MCP4561_Init(void)
{
	I2C1_Init();     //初始化IIC    
}

/************************************************************
*	函数名称：	MCP4561_WriteReg
*	函数功能：	向MCP4561某个寄存器写入数据
*	入口参数：	regAddr    - 寄存器地址: 0~15
*               data       - 待写入的数据
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      
************************************************************/
ErrStatus MCP4561_WriteReg(uint8_t regAddr, uint16_t data)
{
    regAddr = (regAddr<<(MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | ((data>>8)&MCP4561_MSBDATA_MSK) | MCP4561_CMD_WRITE;    
    return I2C1_WriteByte(MCP4561_I2C_ADDR, regAddr, (uint8_t) data);       
}

/************************************************************
*	函数名称：	MCP4561_WriteRandom
*	函数功能：	向MCP4561连续写入多个随机地址数据
*	入口参数：	pRegAddr    - 待写入的地址集合:0~15
*               pData       - 待写入的数据集合
*               size        - 待写入的数据数量
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      pData的高6Bit会被设置为寄存器地址和命令，进行
*               操作后会重新进行恢复
*               输入多少个数据，就应该输入多少个对应的寄存器地址
************************************************************/
ErrStatus MCP4561_WriteRandom(uint8_t *pRegAddr, uint16_t *pData, uint8_t size)
{
    uint8_t i;
    ErrStatus status;
    if(0 == size || size > MCP4561_MEM_SIZE)
        return ERROR;
    
    //交换高低字节(高字节先发)，并插入寄存器和命令
    for(i=0;i<size;i++) 
        pData[i] = (pData[i]<<8) |                          //数据低字节
                   ((pData[i]>>8)&MCP4561_MSBDATA_MSK) |    //数据高bit
                    ((pRegAddr[i]<<MCP4561_MEM_BIT)&MCP4561_MEM_MSK) |         //寄存器地址
                    MCP4561_CMD_WRITE;                      //写命令
    status = I2C1_WriteNByte(MCP4561_I2C_ADDR, (uint8_t *) pData, size<<1);
    //恢复数据
    for(i=0;i<size;i++)
        pData[i] = ((pData[i]<<8)&MCP4561_MSBDATA_MSK) |    //数据高bit
                    (pData[i]>>8);                          //数据低字节
    return status;
}

/************************************************************
*	函数名称：	MCP4561_Write
*	函数功能：	向MCP4561连续写入多个地址连续的数据
*	入口参数：	regAddr     - 待写入的起始地址: 0~15
*               pData       - 待写入的数据集合
*               size        - 待写入的数据数量
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      pData的高6Bit会被设置为寄存器地址和命令，进行
*               操作后会重新进行恢复
************************************************************/
ErrStatus MCP4561_Write(uint8_t regAddr, uint16_t *pData, uint8_t size)
{
    uint8_t i;
    ErrStatus status;
    if(0 == size || size > MCP4561_MEM_SIZE || (regAddr+size) > MCP4561_MEM_SIZE)
        return ERROR;
    
    //交换高低字节(高字节先发)，并插入寄存器和命令
    for(i=0;i<size;i++) 
    {
        pData[i] = (pData[i]<<8) |                          //数据低字节
                   ((pData[i]>>8)&MCP4561_MSBDATA_MSK) |    //数据高bit
                    ((regAddr<<MCP4561_MEM_BIT)&MCP4561_MEM_MSK) |         //寄存器地址
                    MCP4561_CMD_WRITE;                      //写命令
        regAddr++;
    }
    status = I2C1_WriteNByte(MCP4561_I2C_ADDR, (uint8_t *) pData, size<<1);
    //恢复数据
    for(i=0;i<size;i++)
        pData[i] = ((pData[i]<<8)&MCP4561_MSBDATA_MSK) |    //数据高bit
                    (pData[i]>>8);                          //数据低字节
    return status;
}

/************************************************************
*	函数名称：	MCP4561_WriteReg
*	函数功能：	向MCP4561某个寄存器写入数据
*	入口参数：	regAddr    - 寄存器地址: 0~15
*               pData      - 待读出的数据缓冲
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      
************************************************************/
ErrStatus MCP4561_ReadReg(uint8_t regAddr, uint16_t *pData)
{
    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | MCP4561_CMD_READ;      
    if(ERROR == I2C1_ReadNByte(MCP4561_I2C_ADDR, regAddr, (uint8_t *) pData, 2)) //读取2字节，由于高字节先发，所以需要交换高低字节
        return ERROR;
    *pData = (*pData>>8) | (*pData<<8); //交换高低字节
    return SUCCESS;
}

/************************************************************
*	函数名称：	MCP4561_Read
*	函数功能：	从MCP4561多次读取同一地址数据
*	入口参数：	regAddr     - 待读取的起始地址:0~15
*               pData       - 待读取的数据缓冲
*               size        - 待读取的数据数量
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      经验证，MCP4561连续读取，得到的实际为第一输入地址数据，MCP4561地址不会自增
************************************************************/
ErrStatus MCP4561_Read(uint8_t regAddr, uint16_t *pData, uint8_t size)
{
    uint8_t i;
    
    if(0 == size || size > MCP4561_MEM_SIZE)
        return ERROR;
    
    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | MCP4561_CMD_READ; 
    if(ERROR == I2C1_ReadNByte(MCP4561_I2C_ADDR, regAddr, (uint8_t *) pData, size<<1))
        return ERROR;
    
    //交换高低字节
    for(i=0;i<size;i++)
        pData[i] = (pData[i]<<8) | (pData[i]>>8); 
    
    return SUCCESS;
}

/************************************************************
*	函数名称：	MCP4561_Increment
*	函数功能：	MCP4561某个寄存器数据递增
*	入口参数：	regAddr    - 寄存器地址:0~15
*               num        - 递增次数
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      只可用于Volatile Wiper
************************************************************/
ErrStatus MCP4561_Increment(uint8_t regAddr, uint8_t num)
{
    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) |  MCP4561_CMD_INC;    
    return I2C1_WriteMultiple(MCP4561_I2C_ADDR, regAddr, num);       
}

/************************************************************
*	函数名称：	MCP4561_Increment
*	函数功能：	MCP4561某个寄存器数据递减
*	入口参数：	regAddr    - 寄存器地址: 0~15
*               num        - 递减次数
*	返回参数：	成功，返回SUCCESS，否则返回ERROR
*	说明：      只可用于Volatile Wiper
************************************************************/
ErrStatus MCP4561_Decrement(uint8_t regAddr, uint8_t num)
{
    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) |  MCP4561_CMD_DEC;    
    return I2C1_WriteMultiple(MCP4561_I2C_ADDR, regAddr, num);       
}

/** ---	测试代码 ---------------------------------------  **/
/************************************************************
*	函数名称：	MCP4561_DisplayAll
*	函数功能：	打印MCP4561的所有寄存器
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void MCP4561_DisplayAll(void)
{
    uint16_t iRegs[MCP4561_MEM_SIZE];
    uint8_t i;
	ErrStatus iStatus = SUCCESS;
	char ch;
    
    cprintf("\nPrint all registers value of MCP4561\n");
	cprintf("Enter [C] for Continuous Read, Others for One by One Read\n");
	ch = CS_Getch();
	if(ch=='C' || ch=='c')	//连续读取测试
	{
		cprintf("Continuous Reading...\n");
		iStatus = MCP4561_Read(MCP4561_MEM_START, iRegs, MCP4561_MEM_SIZE);				
	}
	else
	{
		cprintf("One by One Reading...\n");
		for(i=0; i<MCP4561_MEM_SIZE; i++)
		{
			if(ERROR == MCP4561_ReadReg( i, &iRegs[i]))
			{
				iStatus = ERROR;
				cprintf("  [ERROR] Read Register: %d\n", i);
				break;
			}
		}				
	}
	
	if(ERROR != iStatus)
	{
		cprintf("  [OK] Read Registers:\n");
		cprintf("  0h: %03X, Volatile Wiper 0\n", iRegs[0]);
		cprintf("  1h: %03X, Volatile Wiper 1\n", iRegs[1]);
		cprintf("  2h: %03X, Non Volatile Wiper 0\n", iRegs[2]);
		cprintf("  3h: %03X, Non Volatile Wiper 1\n", iRegs[3]);
		cprintf("  4h: %03X, Volatile TCON Register\n", iRegs[4]);
		cprintf("  5h: %03X, Status Register\n", iRegs[5]);        
		for(i=MCP4561_MEM_EEPROM_START; i<MCP4561_MEM_SIZE; i++)
			cprintf("  %Xh: %03X, Data EEPROM\n", i, iRegs[i]);  
	}
	else
		cprintf("  [ERROR] Read Registers\n");
    
    cprintf("End of registers Print\n");
}

/************************************************************
*	函数名称：	MCP4561_ReviseRegs
*	函数功能：	修改MCP4561的寄存器值
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void MCP4561_ReviseRegs(void)
{
    int iData = 0;
    uint8_t regAddr = 0;
    uint16_t regData, regDataNew;
    cprintf("\nTest Revise the Registers\n");
    while(1)
    {
        cprintf("Enter the Register Address for Revising(Range: 0~15)(-1 for exit)(Default: %d): ", regAddr);
        if(ERROR != CS_GetIntNum(&iData))
        {
            if(iData>=0 && iData < 16)
                regAddr = (uint8_t) iData;
            else if(iData == -1)
                break;
        }
        cprintf("  Reg Addr: %d\n", regAddr);
        
        if( ERROR == MCP4561_ReadReg( regAddr, &regData))
        {
            cprintf("  [ERROR] Read Reg\n");
            continue;
        }
        
        cprintf("  Old Data: 0x%03X\n", regData);
        cprintf("Enter the Data To Set(Range:0h~1FFh): ");
        if(ERROR != CS_GetIntNum(&iData))
        {
            if(iData<0 || iData>0x1FF)
            {
                cprintf("  Input Out of Range\n");
                continue;
            }
            regData = (uint16_t) iData;
            if( ERROR == MCP4561_WriteReg(regAddr, regData))
            {
                cprintf("  [ERROR] Write Reg\n");
                continue;
            }
            if( ERROR == MCP4561_ReadReg( regAddr, &regDataNew))
            {
                cprintf("  [ERROR] Read Reg Back\n");
                continue;
            }
            if( regDataNew != regData)
                cprintf("  [ERROR] Write Fail, Set: 0x%03X, Read: 0x%03X\n", regData, regDataNew);
            else
                cprintf("  [OK] Write Success\n");
        }
        else
            cprintf("  Nothing Input\n");
    }
    cprintf("End of Revise Register\n");
	CS_WaitKey();
}

/************************************************************
*	函数名称：	MCP4561_SetResistor
*	函数功能：	临时设置电阻值
*	入口参数：	val, 允许范围：0~256
*	返回参数：	None
*	说明：      
************************************************************/
ErrStatus MCP4561_SetResistor(uint16_t val)
{
	uint16_t iRd;
	//写入
	if(ERROR == MCP4561_WriteReg(MCP4561_MEM_VWIPER0, val))
		return ERROR;
	
	//读回
	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &iRd))
		return ERROR;
	if( val != iRd)
		return ERROR;
	return SUCCESS;	
}

/************************************************************
*	函数名称：	MCP4561_SaveResistor
*	函数功能：	保存并设置电阻值
*	入口参数：	val, 允许范围：0~256
*	返回参数：	None
*	说明：      
************************************************************/
ErrStatus MCP4561_SaveResistor(uint16_t val)
{
	uint16_t iRd;
	//设置临时值
	if(ERROR == MCP4561_SetResistor(val))
		return ERROR;
	
	//写入
	if(ERROR == MCP4561_WriteReg(MCP4561_MEM_NVWIPER0, val))
		return ERROR;
	
	//读回
	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_NVWIPER0, &iRd))
		return ERROR;
	if( val != iRd)
		return ERROR;
	return SUCCESS;	
}

/************************************************************
*	函数名称：	MCP4561_GetResistor
*	函数功能：	获取当前的电阻设置
*	入口参数：	None
*	返回参数：	电阻设置值
*	说明：      
************************************************************/
uint16_t MCP4561_GetResistor(void)
{
	uint16_t iRd;
	
	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &iRd))
		iRd = 0xFFFF;
	return iRd;
}



/************************************************************
*	函数名称：	MCP4561_ScanTest
*	函数功能：	对MCP4561按照指定时间进行扫描
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void MCP4561_ScanTest(void)
{
    int iData;
    uint32_t iDelay = 100;
    int16_t regData, regDataNew;
    uint8_t  iStep = 1;
    char ch;
    cprintf("\nTest MCP4561 Scan\n");
    cprintf("Enter the Interval Time for 1 change(range: 1~1000ms)(Default: %dms): ", iDelay);
    if(ERROR != CS_GetIntNum(&iData))
    {
        if( iData>=1 && iData<=1000)
            iDelay = (uint32_t) iData;
    }    
    cprintf("  Set Interval Time: %dms\n", iDelay);
    
    cprintf("Enter the Steps for 1 change(range: 1~128)(Default: %d): ", iStep);
    if(ERROR != CS_GetIntNum(&iData))
    {
        if( iData>=1 && iData<=128)
            iStep = (uint16_t) iData;
    }
    cprintf("  Set Steps: %d\n", iStep);
    
    if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, (uint16_t *)&regData))
    {
        cprintf("  [ERROR] Read Volatile Wiper0, Exit\n");
        return;
    }   
    cprintf("  Initial WIPER0 Data: 0x%03X\n", regData);
    cprintf("Scan Now, Press Any Key to Stop...\n");
    iData = 1;  //增长
    while(ERROR == CS_QueryChar(&ch))
    {
        if( iData==1 && regData<MCP4561_STEP_MAX) //增长
        {
            if(ERROR == MCP4561_Increment(MCP4561_MEM_VWIPER0, iStep))
            {
                cprintf("  [ERROR] Increment\n");
                break;
            }
            regData += iStep;
            if(regData >= MCP4561_STEP_MAX)
            {
                regData = MCP4561_STEP_MAX;
                iData = 0;  //下次减
            }
        }
        else    //减小
        {
            if(ERROR == MCP4561_Decrement(MCP4561_MEM_VWIPER0, iStep))
            {
                cprintf("  [ERROR] Decrement\n");
                break;
            }
            regData -= iStep;
            if(regData <= 0)
            {
                regData = 0;
                iData = 1;
            }
            else                
                iData = 0;
        }         
        delay_1ms(iDelay);
    }
    
    if(ERROR != MCP4561_ReadReg(MCP4561_MEM_VWIPER0, (uint16_t *)&regDataNew))
    {
        if( regDataNew != regData)
            cprintf("  [ERROR] Not Agreement, Expected: 0x%03X, Read: 0x%03X\n", regData, regDataNew);
        else
            cprintf("  [OK] The inc&dec Data agreement, Set As: 0x%03X\n", regData);
    } 
    else
        cprintf("  [ERROR] Read Volatitle Wiper0 Register\n");
    
    cprintf("End of Scan Test\n");
}

void MCP4561_ManualAdjust(void)
{
	int16_t iStep = 1;
	uint16_t orgStep;
	int16_t  iCnt;
	
	int iInput;
	char ch = 0;
	
	cprintf("\nAdjust Compensation Amplitude Manually\n");
	cprintf("Enter the Step for Adjust(Range: 1~32)(Default: %d): ", iStep);
	if(ERROR != CS_GetIntNum(&iInput))
	{
		if(iInput>=1 && iInput<=32)
			iStep = iInput;
	}
	cprintf("  Set Step As: %d\n", iStep);
	
	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &orgStep))
	{
		cprintf("  [ERROR] Read MCP4561 VWIPER0\n");
		return;
	}
	
	iCnt = (int16_t) orgStep;
	cprintf("Original Amplitude is: %d\n", iCnt);
	cprintf("Press [+] for Up Step And [-] for Down Step, Others Exit\n");
	while(1)
	{
		if(ERROR == CS_QueryChar(&ch))
			continue;
		if(ch=='+')
		{
			iCnt += iStep;
			if(iCnt > 256)
				iCnt = 256;
		}
		else if(ch=='-')
		{
			iCnt -= iStep;
			if(iCnt < 0)
				iCnt = 0;
		}
		else
			break;	
		
		if(ERROR == MCP4561_SetResistor((uint16_t) iCnt))
		{
			cprintf("  [ERROR] Write Resistor\n");
			break;
		}			
		cprintf("  AMP: %d\n", iCnt);
	}	
	cprintf("End of Manual Copensation Adjust\n");
	CS_WaitKey();
}

/************************************************************
*	函数名称：	MCP4561_Test
*	函数功能：	显示MCP4561测试目录
*	入口参数：	None
*	返回参数：	None
*	说明：      
************************************************************/
void MCP4561_Test(void)
{
    cprintf("\nTest MCP4561 Functions\n");
    MENU_Disp(func_mcp4561_test); 
    cprintf("End of MCP4561 Function Test\n");
}

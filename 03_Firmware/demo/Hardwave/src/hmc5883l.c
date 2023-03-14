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

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	оƬ�������� -----------------------------------  **/

static int i2cWrite(uint8_t reg_, uint8_t Data)
{
	//return i2c_mem_write(MAG_ADDRESS, reg_, &Data, 1);
	return HAL_I2C_Mem_Write( &hi2c1, HMC5883_WRITE_ADDR, reg_, 1, &Data, 1, 100);
}

static int i2cRead(uint8_t reg_, uint8_t len, uint8_t* buf)
{
	//return i2c_mem_read(MAG_ADDRESS, reg_, buf, len);
	return HAL_I2C_Mem_Read( &hi2c1, HMC5883_READ_ADDR, reg_, 1, buf, len, 100);
}

/**
* @brief	hmc5883l��ʼ��,����������У׼
* @note
*/
void HMC5883l_Init(void)
{
	i2cWrite(HMC5883_REG_CONFIGA, HMC5883_REG_CRA_MA_AVG_8 | HMC5883_REG_CRA_DO_Hz_75 | HMC5883_REG_CRA_MS_NORAL);
	i2cWrite(HMC5883_REG_CONFIGB, HMC5883_REG_GN_G820);
	i2cWrite(HMC5883_REG_MODE, HMC5883_REG_MODE_MR_CON);	
}

/**
* @brief	��ȡ�ų����������ݡ�
* @note
* @param	magData���洢�ų�������ԭʼ���ݵ�ָ��
*/
void HMC5883l_Read(int16_t *mag_x, int16_t *mag_y, int16_t *mag_z)
{
	uint8_t buf[6];
	i2cRead(HMC5883_REG_X_MSB, 6, buf);
	*mag_x = buf[0] << 8 | buf[1];
	*mag_z = buf[2] << 8 | buf[3];
	*mag_y = buf[4] << 8 | buf[5];
}








///** ---	Start of Macro define 	---------------------------------------------------------------------  **/
//#define MCP4561_I2C_ADDR        (0x5C)  //MCP45x1���豸��ַ��0101 11 A0 0 = 0x5c, A0���Žӵ� ����A0 = 0 
//#define MCP4561_STEP_MAX        (0x100) //�����

////Command Operation bits
//#define MCP4561_CMD_MSK         (0x0C)
//#define MCP4561_CMD_WRITE       (0x00)  //Write Data
//#define MCP4561_CMD_READ        (0x0C)  //Read Data
//#define MCP4561_CMD_INC         (0x04)  //Incremen, Only for volatile
//#define MCP4561_CMD_DEC         (0x08)  //Decrement, Only for volatile

////MSB Data
//#define MCP4561_MSBDATA_MSK     (0x03)

////Memory Address
//#define MCP4561_MEM_MSK         (0xF0)
//#define MCP4561_MEM_BIT         (4u)    //MEM Address bit position
//#define MCP4561_MEM_SIZE        (0x10)  //MEM Size
//#define MCP4561_MEM_START       (0x0)   //The Start Address of registers
//#define MCP4561_MEM_VWIPER0     (0x0)   //Volatile Wiper0
//#define MCP4561_MEM_VWIPER1     (0x1)   //Volatile Wiper1
//#define MCP4561_MEM_NVWIPER0    (0x2)   //None Volatile Wiper0
//#define MCP4561_MEM_NVWIPER1    (0x3)   //None Volatile Wiper1
//#define MCP4561_MEM_VTCON       (0x4)   //Volatile TCON Register
//#define MCP4561_MEM_STATUS      (0x5)   //Status Register
//#define MCP4561_MEM_EEPROM_START (0x6)  //Data EEPROM Start Addr
//#define MCP4561_MEM_EEPROM_END  (0xF)   //Data EEPROM End Addr

////Status Bit
//#define BITMSK_STATUS_WP        (0x01)  //1: EEPROMд������0����д����
//#define BITMSK_STATUS_WIL0      (0x02)  //1: ��λ��0��Wiper��TCONд������0����д����
//#define BITMSK_STATUS_WIL1      (0x04)  //1: ��λ��1��Wiper��TCONд������0����д����
//#define BITMSK_STATUS_EEWA      (0x08)  //1: ����д��EEPROM��0��δд������

////TCON Bit: �ϵ�Ĭ��0x1FF
//#define BITMSK_TCON_GCEN        (0x100) //1: ʹ�ܽ���ͨ�ú��е�ַ(0000h); 0: ����
//#define BITMSK_TCON_R1HW        (0x80)  //1: ��λ��1û��ǿ��ΪӲ��Shutdown���ã�0��ǿ��ΪӲ��Shutdown
//#define BITMSK_TCON_R1A         (0x40)  //1: P1A�����λ����0���Ͽ�
//#define BITMSK_TCON_R1W         (0x20)  //1: P1W�����λ����0���Ͽ�
//#define BITMSK_TCON_R1B         (0x10)  //1: P1B�����λ����0���Ͽ�
//#define BITMSK_TCON_R0HW        (0x08)  //1: ��λ��0û��ǿ��ΪӲ��Shutdown���ã�0��ǿ��ΪӲ��Shutdown
//#define BITMSK_TCON_R0A         (0x04)  //1: P0A�����λ����0���Ͽ�
//#define BITMSK_TCON_R0W         (0x02)  //1: P0W�����λ����0���Ͽ�
//#define BITMSK_TCON_R0B         (0x01)  //1: P0B�����λ����0���Ͽ�

///** ---	Start of Para define 	---------------------------------------------------------------------  **/
//void MCP4561_DisplayAll(void);
//void MCP4561_ReviseRegs(void);
//void MCP4561_ScanTest(void);

//tdCS_TestList func_mcp4561_test[]=
//{
//	{MCP4561_DisplayAll,   "Display All Registers"         },
//	{MCP4561_ReviseRegs,   "Modify specified Register"     },
//	{MCP4561_ManualAdjust, "Compensate Amp Manual Adjust"  },
//    {MCP4561_ScanTest,     "Test Scan the resistor"        },
//	{0,0}
//};

///** ---	Start of Funtion define ---------------------------------------------------------------------  **/
///** ---	оƬ�������� -----------------------------------  **/
///************************************************************
//*	�������ƣ�	MCP4561_Init
//*	�������ܣ�	��ʼ����λ��
//*	��ڲ�����	None
//*	���ز�����	None
//*	˵����      
//************************************************************/
//void MCP4561_Init(void)
//{
//	I2C1_Init();     //��ʼ��IIC    
//}

///************************************************************
//*	�������ƣ�	MCP4561_WriteReg
//*	�������ܣ�	��MCP4561ĳ���Ĵ���д������
//*	��ڲ�����	regAddr    - �Ĵ�����ַ: 0~15
//*               data       - ��д�������
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      
//************************************************************/
//ErrStatus MCP4561_WriteReg(uint8_t regAddr, uint16_t data)
//{
//    regAddr = (regAddr<<(MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | ((data>>8)&MCP4561_MSBDATA_MSK) | MCP4561_CMD_WRITE;    
//    return I2C1_WriteByte(MCP4561_I2C_ADDR, regAddr, (uint8_t) data);       
//}

///************************************************************
//*	�������ƣ�	MCP4561_WriteRandom
//*	�������ܣ�	��MCP4561����д���������ַ����
//*	��ڲ�����	pRegAddr    - ��д��ĵ�ַ����:0~15
//*               pData       - ��д������ݼ���
//*               size        - ��д�����������
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      pData�ĸ�6Bit�ᱻ����Ϊ�Ĵ�����ַ���������
//*               ����������½��лָ�
//*               ������ٸ����ݣ���Ӧ��������ٸ���Ӧ�ļĴ�����ַ
//************************************************************/
//ErrStatus MCP4561_WriteRandom(uint8_t *pRegAddr, uint16_t *pData, uint8_t size)
//{
//    uint8_t i;
//    ErrStatus status;
//    if(0 == size || size > MCP4561_MEM_SIZE)
//        return ERROR;
//    
//    //�����ߵ��ֽ�(���ֽ��ȷ�)��������Ĵ���������
//    for(i=0;i<size;i++) 
//        pData[i] = (pData[i]<<8) |                          //���ݵ��ֽ�
//                   ((pData[i]>>8)&MCP4561_MSBDATA_MSK) |    //���ݸ�bit
//                    ((pRegAddr[i]<<MCP4561_MEM_BIT)&MCP4561_MEM_MSK) |         //�Ĵ�����ַ
//                    MCP4561_CMD_WRITE;                      //д����
//    status = I2C1_WriteNByte(MCP4561_I2C_ADDR, (uint8_t *) pData, size<<1);
//    //�ָ�����
//    for(i=0;i<size;i++)
//        pData[i] = ((pData[i]<<8)&MCP4561_MSBDATA_MSK) |    //���ݸ�bit
//                    (pData[i]>>8);                          //���ݵ��ֽ�
//    return status;
//}

///************************************************************
//*	�������ƣ�	MCP4561_Write
//*	�������ܣ�	��MCP4561����д������ַ����������
//*	��ڲ�����	regAddr     - ��д�����ʼ��ַ: 0~15
//*               pData       - ��д������ݼ���
//*               size        - ��д�����������
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      pData�ĸ�6Bit�ᱻ����Ϊ�Ĵ�����ַ���������
//*               ����������½��лָ�
//************************************************************/
//ErrStatus MCP4561_Write(uint8_t regAddr, uint16_t *pData, uint8_t size)
//{
//    uint8_t i;
//    ErrStatus status;
//    if(0 == size || size > MCP4561_MEM_SIZE || (regAddr+size) > MCP4561_MEM_SIZE)
//        return ERROR;
//    
//    //�����ߵ��ֽ�(���ֽ��ȷ�)��������Ĵ���������
//    for(i=0;i<size;i++) 
//    {
//        pData[i] = (pData[i]<<8) |                          //���ݵ��ֽ�
//                   ((pData[i]>>8)&MCP4561_MSBDATA_MSK) |    //���ݸ�bit
//                    ((regAddr<<MCP4561_MEM_BIT)&MCP4561_MEM_MSK) |         //�Ĵ�����ַ
//                    MCP4561_CMD_WRITE;                      //д����
//        regAddr++;
//    }
//    status = I2C1_WriteNByte(MCP4561_I2C_ADDR, (uint8_t *) pData, size<<1);
//    //�ָ�����
//    for(i=0;i<size;i++)
//        pData[i] = ((pData[i]<<8)&MCP4561_MSBDATA_MSK) |    //���ݸ�bit
//                    (pData[i]>>8);                          //���ݵ��ֽ�
//    return status;
//}

///************************************************************
//*	�������ƣ�	MCP4561_WriteReg
//*	�������ܣ�	��MCP4561ĳ���Ĵ���д������
//*	��ڲ�����	regAddr    - �Ĵ�����ַ: 0~15
//*               pData      - �����������ݻ���
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      
//************************************************************/
//ErrStatus MCP4561_ReadReg(uint8_t regAddr, uint16_t *pData)
//{
//    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | MCP4561_CMD_READ;      
//    if(ERROR == I2C1_ReadNByte(MCP4561_I2C_ADDR, regAddr, (uint8_t *) pData, 2)) //��ȡ2�ֽڣ����ڸ��ֽ��ȷ���������Ҫ�����ߵ��ֽ�
//        return ERROR;
//    *pData = (*pData>>8) | (*pData<<8); //�����ߵ��ֽ�
//    return SUCCESS;
//}

///************************************************************
//*	�������ƣ�	MCP4561_Read
//*	�������ܣ�	��MCP4561��ζ�ȡͬһ��ַ����
//*	��ڲ�����	regAddr     - ����ȡ����ʼ��ַ:0~15
//*               pData       - ����ȡ�����ݻ���
//*               size        - ����ȡ����������
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      ����֤��MCP4561������ȡ���õ���ʵ��Ϊ��һ�����ַ���ݣ�MCP4561��ַ��������
//************************************************************/
//ErrStatus MCP4561_Read(uint8_t regAddr, uint16_t *pData, uint8_t size)
//{
//    uint8_t i;
//    
//    if(0 == size || size > MCP4561_MEM_SIZE)
//        return ERROR;
//    
//    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) | MCP4561_CMD_READ; 
//    if(ERROR == I2C1_ReadNByte(MCP4561_I2C_ADDR, regAddr, (uint8_t *) pData, size<<1))
//        return ERROR;
//    
//    //�����ߵ��ֽ�
//    for(i=0;i<size;i++)
//        pData[i] = (pData[i]<<8) | (pData[i]>>8); 
//    
//    return SUCCESS;
//}

///************************************************************
//*	�������ƣ�	MCP4561_Increment
//*	�������ܣ�	MCP4561ĳ���Ĵ������ݵ���
//*	��ڲ�����	regAddr    - �Ĵ�����ַ:0~15
//*               num        - ��������
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      ֻ������Volatile Wiper
//************************************************************/
//ErrStatus MCP4561_Increment(uint8_t regAddr, uint8_t num)
//{
//    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) |  MCP4561_CMD_INC;    
//    return I2C1_WriteMultiple(MCP4561_I2C_ADDR, regAddr, num);       
//}

///************************************************************
//*	�������ƣ�	MCP4561_Increment
//*	�������ܣ�	MCP4561ĳ���Ĵ������ݵݼ�
//*	��ڲ�����	regAddr    - �Ĵ�����ַ: 0~15
//*               num        - �ݼ�����
//*	���ز�����	�ɹ�������SUCCESS�����򷵻�ERROR
//*	˵����      ֻ������Volatile Wiper
//************************************************************/
//ErrStatus MCP4561_Decrement(uint8_t regAddr, uint8_t num)
//{
//    regAddr = ((regAddr<<MCP4561_MEM_BIT) & MCP4561_MEM_MSK) |  MCP4561_CMD_DEC;    
//    return I2C1_WriteMultiple(MCP4561_I2C_ADDR, regAddr, num);       
//}

///** ---	���Դ��� ---------------------------------------  **/
///************************************************************
//*	�������ƣ�	MCP4561_DisplayAll
//*	�������ܣ�	��ӡMCP4561�����мĴ���
//*	��ڲ�����	None
//*	���ز�����	None
//*	˵����      
//************************************************************/
//void MCP4561_DisplayAll(void)
//{
//    uint16_t iRegs[MCP4561_MEM_SIZE];
//    uint8_t i;
//	ErrStatus iStatus = SUCCESS;
//	char ch;
//    
//    cprintf("\nPrint all registers value of MCP4561\n");
//	cprintf("Enter [C] for Continuous Read, Others for One by One Read\n");
//	ch = CS_Getch();
//	if(ch=='C' || ch=='c')	//������ȡ����
//	{
//		cprintf("Continuous Reading...\n");
//		iStatus = MCP4561_Read(MCP4561_MEM_START, iRegs, MCP4561_MEM_SIZE);				
//	}
//	else
//	{
//		cprintf("One by One Reading...\n");
//		for(i=0; i<MCP4561_MEM_SIZE; i++)
//		{
//			if(ERROR == MCP4561_ReadReg( i, &iRegs[i]))
//			{
//				iStatus = ERROR;
//				cprintf("  [ERROR] Read Register: %d\n", i);
//				break;
//			}
//		}				
//	}
//	
//	if(ERROR != iStatus)
//	{
//		cprintf("  [OK] Read Registers:\n");
//		cprintf("  0h: %03X, Volatile Wiper 0\n", iRegs[0]);
//		cprintf("  1h: %03X, Volatile Wiper 1\n", iRegs[1]);
//		cprintf("  2h: %03X, Non Volatile Wiper 0\n", iRegs[2]);
//		cprintf("  3h: %03X, Non Volatile Wiper 1\n", iRegs[3]);
//		cprintf("  4h: %03X, Volatile TCON Register\n", iRegs[4]);
//		cprintf("  5h: %03X, Status Register\n", iRegs[5]);        
//		for(i=MCP4561_MEM_EEPROM_START; i<MCP4561_MEM_SIZE; i++)
//			cprintf("  %Xh: %03X, Data EEPROM\n", i, iRegs[i]);  
//	}
//	else
//		cprintf("  [ERROR] Read Registers\n");
//    
//    cprintf("End of registers Print\n");
//}

///************************************************************
//*	�������ƣ�	MCP4561_ReviseRegs
//*	�������ܣ�	�޸�MCP4561�ļĴ���ֵ
//*	��ڲ�����	None
//*	���ز�����	None
//*	˵����      
//************************************************************/
//void MCP4561_ReviseRegs(void)
//{
//    int iData = 0;
//    uint8_t regAddr = 0;
//    uint16_t regData, regDataNew;
//    cprintf("\nTest Revise the Registers\n");
//    while(1)
//    {
//        cprintf("Enter the Register Address for Revising(Range: 0~15)(-1 for exit)(Default: %d): ", regAddr);
//        if(ERROR != CS_GetIntNum(&iData))
//        {
//            if(iData>=0 && iData < 16)
//                regAddr = (uint8_t) iData;
//            else if(iData == -1)
//                break;
//        }
//        cprintf("  Reg Addr: %d\n", regAddr);
//        
//        if( ERROR == MCP4561_ReadReg( regAddr, &regData))
//        {
//            cprintf("  [ERROR] Read Reg\n");
//            continue;
//        }
//        
//        cprintf("  Old Data: 0x%03X\n", regData);
//        cprintf("Enter the Data To Set(Range:0h~1FFh): ");
//        if(ERROR != CS_GetIntNum(&iData))
//        {
//            if(iData<0 || iData>0x1FF)
//            {
//                cprintf("  Input Out of Range\n");
//                continue;
//            }
//            regData = (uint16_t) iData;
//            if( ERROR == MCP4561_WriteReg(regAddr, regData))
//            {
//                cprintf("  [ERROR] Write Reg\n");
//                continue;
//            }
//            if( ERROR == MCP4561_ReadReg( regAddr, &regDataNew))
//            {
//                cprintf("  [ERROR] Read Reg Back\n");
//                continue;
//            }
//            if( regDataNew != regData)
//                cprintf("  [ERROR] Write Fail, Set: 0x%03X, Read: 0x%03X\n", regData, regDataNew);
//            else
//                cprintf("  [OK] Write Success\n");
//        }
//        else
//            cprintf("  Nothing Input\n");
//    }
//    cprintf("End of Revise Register\n");
//	CS_WaitKey();
//}

///************************************************************
//*	�������ƣ�	MCP4561_SetResistor
//*	�������ܣ�	��ʱ���õ���ֵ
//*	��ڲ�����	val, ����Χ��0~256
//*	���ز�����	None
//*	˵����      
//************************************************************/
//ErrStatus MCP4561_SetResistor(uint16_t val)
//{
//	uint16_t iRd;
//	//д��
//	if(ERROR == MCP4561_WriteReg(MCP4561_MEM_VWIPER0, val))
//		return ERROR;
//	
//	//����
//	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &iRd))
//		return ERROR;
//	if( val != iRd)
//		return ERROR;
//	return SUCCESS;	
//}

///************************************************************
//*	�������ƣ�	MCP4561_SaveResistor
//*	�������ܣ�	���沢���õ���ֵ
//*	��ڲ�����	val, ����Χ��0~256
//*	���ز�����	None
//*	˵����      
//************************************************************/
//ErrStatus MCP4561_SaveResistor(uint16_t val)
//{
//	uint16_t iRd;
//	//������ʱֵ
//	if(ERROR == MCP4561_SetResistor(val))
//		return ERROR;
//	
//	//д��
//	if(ERROR == MCP4561_WriteReg(MCP4561_MEM_NVWIPER0, val))
//		return ERROR;
//	
//	//����
//	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_NVWIPER0, &iRd))
//		return ERROR;
//	if( val != iRd)
//		return ERROR;
//	return SUCCESS;	
//}

///************************************************************
//*	�������ƣ�	MCP4561_GetResistor
//*	�������ܣ�	��ȡ��ǰ�ĵ�������
//*	��ڲ�����	None
//*	���ز�����	��������ֵ
//*	˵����      
//************************************************************/
//uint16_t MCP4561_GetResistor(void)
//{
//	uint16_t iRd;
//	
//	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &iRd))
//		iRd = 0xFFFF;
//	return iRd;
//}



///************************************************************
//*	�������ƣ�	MCP4561_ScanTest
//*	�������ܣ�	��MCP4561����ָ��ʱ�����ɨ��
//*	��ڲ�����	None
//*	���ز�����	None
//*	˵����      
//************************************************************/
//void MCP4561_ScanTest(void)
//{
//    int iData;
//    uint32_t iDelay = 100;
//    int16_t regData, regDataNew;
//    uint8_t  iStep = 1;
//    char ch;
//    cprintf("\nTest MCP4561 Scan\n");
//    cprintf("Enter the Interval Time for 1 change(range: 1~1000ms)(Default: %dms): ", iDelay);
//    if(ERROR != CS_GetIntNum(&iData))
//    {
//        if( iData>=1 && iData<=1000)
//            iDelay = (uint32_t) iData;
//    }    
//    cprintf("  Set Interval Time: %dms\n", iDelay);
//    
//    cprintf("Enter the Steps for 1 change(range: 1~128)(Default: %d): ", iStep);
//    if(ERROR != CS_GetIntNum(&iData))
//    {
//        if( iData>=1 && iData<=128)
//            iStep = (uint16_t) iData;
//    }
//    cprintf("  Set Steps: %d\n", iStep);
//    
//    if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, (uint16_t *)&regData))
//    {
//        cprintf("  [ERROR] Read Volatile Wiper0, Exit\n");
//        return;
//    }   
//    cprintf("  Initial WIPER0 Data: 0x%03X\n", regData);
//    cprintf("Scan Now, Press Any Key to Stop...\n");
//    iData = 1;  //����
//    while(ERROR == CS_QueryChar(&ch))
//    {
//        if( iData==1 && regData<MCP4561_STEP_MAX) //����
//        {
//            if(ERROR == MCP4561_Increment(MCP4561_MEM_VWIPER0, iStep))
//            {
//                cprintf("  [ERROR] Increment\n");
//                break;
//            }
//            regData += iStep;
//            if(regData >= MCP4561_STEP_MAX)
//            {
//                regData = MCP4561_STEP_MAX;
//                iData = 0;  //�´μ�
//            }
//        }
//        else    //��С
//        {
//            if(ERROR == MCP4561_Decrement(MCP4561_MEM_VWIPER0, iStep))
//            {
//                cprintf("  [ERROR] Decrement\n");
//                break;
//            }
//            regData -= iStep;
//            if(regData <= 0)
//            {
//                regData = 0;
//                iData = 1;
//            }
//            else                
//                iData = 0;
//        }         
//        delay_1ms(iDelay);
//    }
//    
//    if(ERROR != MCP4561_ReadReg(MCP4561_MEM_VWIPER0, (uint16_t *)&regDataNew))
//    {
//        if( regDataNew != regData)
//            cprintf("  [ERROR] Not Agreement, Expected: 0x%03X, Read: 0x%03X\n", regData, regDataNew);
//        else
//            cprintf("  [OK] The inc&dec Data agreement, Set As: 0x%03X\n", regData);
//    } 
//    else
//        cprintf("  [ERROR] Read Volatitle Wiper0 Register\n");
//    
//    cprintf("End of Scan Test\n");
//}

//void MCP4561_ManualAdjust(void)
//{
//	int16_t iStep = 1;
//	uint16_t orgStep;
//	int16_t  iCnt;
//	
//	int iInput;
//	char ch = 0;
//	
//	cprintf("\nAdjust Compensation Amplitude Manually\n");
//	cprintf("Enter the Step for Adjust(Range: 1~32)(Default: %d): ", iStep);
//	if(ERROR != CS_GetIntNum(&iInput))
//	{
//		if(iInput>=1 && iInput<=32)
//			iStep = iInput;
//	}
//	cprintf("  Set Step As: %d\n", iStep);
//	
//	if(ERROR == MCP4561_ReadReg(MCP4561_MEM_VWIPER0, &orgStep))
//	{
//		cprintf("  [ERROR] Read MCP4561 VWIPER0\n");
//		return;
//	}
//	
//	iCnt = (int16_t) orgStep;
//	cprintf("Original Amplitude is: %d\n", iCnt);
//	cprintf("Press [+] for Up Step And [-] for Down Step, Others Exit\n");
//	while(1)
//	{
//		if(ERROR == CS_QueryChar(&ch))
//			continue;
//		if(ch=='+')
//		{
//			iCnt += iStep;
//			if(iCnt > 256)
//				iCnt = 256;
//		}
//		else if(ch=='-')
//		{
//			iCnt -= iStep;
//			if(iCnt < 0)
//				iCnt = 0;
//		}
//		else
//			break;	
//		
//		if(ERROR == MCP4561_SetResistor((uint16_t) iCnt))
//		{
//			cprintf("  [ERROR] Write Resistor\n");
//			break;
//		}			
//		cprintf("  AMP: %d\n", iCnt);
//	}	
//	cprintf("End of Manual Copensation Adjust\n");
//	CS_WaitKey();
//}

///************************************************************
//*	�������ƣ�	MCP4561_Test
//*	�������ܣ�	��ʾMCP4561����Ŀ¼
//*	��ڲ�����	None
//*	���ز�����	None
//*	˵����      
//************************************************************/
//void MCP4561_Test(void)
//{
//    cprintf("\nTest MCP4561 Functions\n");
//    MENU_Disp(func_mcp4561_test); 
//    cprintf("End of MCP4561 Function Test\n");
//}

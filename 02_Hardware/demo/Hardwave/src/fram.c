/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               fram.c
** Latest modified Date:    2022-5-25
** Latest Version:          1.0
** Descriptions:            Used for Fram Test(SPI0)
**							FM25L16B(2Kx8), Max. 20MHz
**                          PCLK2: AHB/2 = 84MHz; SPI CLK: 84MHz/8 = 10.5MHz 
**                          PA4 - NSS
**                          PA5 - SCK
**                          PA6 - MISO
**                          PA7 - MOSI
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
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "gd32f4xx.h"
#include "systick.h"

#include "fram.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "ads131m.h"
#include "MCP4561.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
#define FRAM_TIMEOUT_VALUE	        (1000)
#define FRAM_SIZE					(0x800)			//2k*8bit FM25L16
#define FRAM_ADDR_MSK				(FRAM_SIZE-1)

#define FRAM_ADDR_CALI				(FRAM_SIZE>>1)	//�궨���ݵ�ַ
#define FRAM_ADDR_CNT				(0)				//�������ݵ�ַ
#define FRAM_ADDR_CORRECT			(256)		
#define FRAM_CNT_SIZE				(64)			//����64bytes����Լ����洢�����л���
#define FRAM_CORRECT_SIZE			(32)

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

extern tCalInfo g_stPCal;
extern tParticleCnt g_stPCnt;
extern tCorrectInfo g_stCorrect;

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
void FRAM_TestAll(void);
void FRAM_DisplayAll(void);
void FRAM_Modify(void);
void FRAM_ClearCount(void);
void FRAM_SetCalInfo(void);
void FRAM_UpdateCnt(void);
void FRAM_UpdateCorrectInfo(void);
    
tdCS_TestList func_fram_test[]=
{
	{FRAM_DisplayAll,   "Display All Datas in FRAM"     },
	{FRAM_Modify,	    "Modify specified Address"      },
	{FRAM_TestAll,      "Write&Read All of FRAM"        },
	{FRAM_ClearCount,	"Clear particle Counter"		},
	{FRAM_SetCalInfo,	"Set particle Calibrate info"   },
	{FRAM_UpdateCnt,	"Update the Counter to FRAM"    },
	{FRAM_UpdateCorrectInfo,"Update the Correct to FRAM"    },
	{0,0}
};

ErrStatus FRAM_Read(uint8_t *pData, uint16_t iAddr, uint16_t iSize, uint8_t bCrc);

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	FRAM_CheckSum
*	�������ܣ�	�˶�У���
*	��ڲ�����	pu8Data	- ����У��͵����ݻ���
*				iSize	- �������͵����ݶ���
*				sum     - ���˶Եļ����
*	���ز�����	����ɹ�������SUCCESS������ʧ�ܣ�����ERROR;
*	˵����      У��� Ϊ ��������֮��ȡ�����Է�ֹȫ����0���
************************************************************/
ErrStatus FRAM_CheckSum(uint8_t *pu8Data, int iSize, uint8_t sum)
{
	uint8_t u8Sum = 0;	
	while(iSize-->0)
		u8Sum += *pu8Data++;
	return (u8Sum == ~sum)?SUCCESS:ERROR;
}

/************************************************************
*	�������ƣ�	FRAM_CalcSum
*	�������ܣ�	�˶�У���
*	��ڲ�����	pu8Data	- ����У��͵����ݻ���
*				iSize	- �������͵����ݶ���
*	���ز�����	������õ�У���
*	˵����      У��� Ϊ ��������֮��ȡ�����Է�ֹȫ����0���
************************************************************/
uint8_t FRAM_CalcSum(uint8_t *pu8Data, int iSize)
{
	uint8_t sum=0;
	while(iSize-->0)
		sum += *pu8Data++;
	return ~sum;
}

/************************************************************
*	�������ƣ�	FRAM_Init
*	�������ܣ�	��ʼ��FRAM�ӿ�
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_Init(void)
{
	tParticleCnt stPCnt, *pstCnt;
	int i, bCntOk;
	uint16_t addr;
	
    SPI0_Init();
	
	//��ȡ�궨��Ϣ
	if((ERROR == FRAM_Read((uint8_t *) &g_stPCal, FRAM_ADDR_CALI, sizeof(tCalInfo), 1)) 
		|| (g_stPCal.magic != ('W'<<24 | 'R'<<16 | 'D'<<8 | 'O')) || ERROR == FRAM_CheckSum((uint8_t *) &g_stPCal, sizeof(tCalInfo)-2, g_stPCal.sum))
	{//��ȡ�궨��Ϣ�������ض�һ��		
		if( (ERROR == FRAM_Read((uint8_t *) &g_stPCal, FRAM_ADDR_CALI, sizeof(tCalInfo), 1)) 
			|| (g_stPCal.magic != ('W'<<24 | 'R'<<16 | 'D'<<8 | 'O')) || ERROR == FRAM_CheckSum((uint8_t *) &g_stPCal, sizeof(tCalInfo)-2, g_stPCal.sum))
		{//�ٴδ��������³�ʼ��
			g_stPCal.magic = 'W'<<24 | 'R'<<16 | 'D'<<8 | 'O';
			g_stPCal.ver   = 0x1<<6;	//V0.1.0
			g_stPCal.productSnH  = 91010022;
			g_stPCal.productSnL  = 10000000;
			g_stPCal.mainSnH     = 0122;
			g_stPCal.mainSnL	 = 10000000;
			g_stPCal.comSnH		 = 0122;
			g_stPCal.comSnL		 = 10000000;
			g_stPCal.powerSnH	 = 0122;
			g_stPCal.powerSnL	 = 10000000;
			//time(&(g_stPCal.date));
			memset((void*) &(g_stPCal.cu_acal[0]), 0xFF, 80);	//�궨������Ϊ���FF
			g_stPCal.sum = 0;
			g_stPCal.crc = 0;
		}
	}
	
	//��ȡУ׼��Ϣ
	addr = FRAM_ADDR_CORRECT;
	bCntOk = 0;
	for(i=0;i<4;i++)
	{
		if( ERROR != FRAM_Read((uint8_t *) &g_stCorrect, addr, sizeof(tCorrectInfo), 1)						//��ȡ�ɹ�
			&& ERROR != FRAM_CheckSum((uint8_t *) &g_stCorrect, sizeof(tCorrectInfo)-2, g_stCorrect.sum))	//У��ɹ�
		{
			bCntOk = 1;
			break;
		}			
		addr += FRAM_CORRECT_SIZE;
	}
	if(bCntOk == 0)	//δ��ȡ����Ч���ݣ����г�ʼ��
	{
		memset((void *) &g_stCorrect, 0, sizeof(tCorrectInfo));
		g_stCorrect.cpsPhase = 0;
		g_stCorrect.cpsRes = MCP4561_FULL_RANGE/8;
		g_stCorrect.noise = 20;
	}			
	
	//��ȡ������Ϣ
	bCntOk = 0;
	pstCnt = &g_stPCnt;	//g_stPCntΪǰ��
	addr = FRAM_ADDR_CNT;
	for(i=0;i<4;i++)
	{
		if(ERROR != FRAM_Read((uint8_t *) pstCnt, addr, sizeof(tParticleCnt), 1) 
			&& (pstCnt->number&0x3)==i && ERROR != FRAM_CheckSum((uint8_t *) pstCnt, sizeof(tParticleCnt)-2, pstCnt->sum))	//У��ɹ�
		{
			if(bCntOk)	//�Ѿ�������������
			{
				if( (stPCnt.number > g_stPCnt.number && (stPCnt.number - g_stPCnt.number)<4) ||	//��������ǰ��, ����������
					(stPCnt.number < g_stPCnt.number && (g_stPCnt.number - stPCnt.number)>4))		//ǰ�����ں��������������ϴ�
				{
					memcpy((void *) &g_stPCnt, (void *) &stPCnt, sizeof(tParticleCnt));	//�Ժ���Ϊ��׼����Ѱ��							
				}
			}
			else
				pstCnt = &stPCnt;	//stPCnt Ϊ����
			bCntOk = 1;				
		}			
		addr += FRAM_CNT_SIZE;
	}
	
	if(bCntOk==0)	//δ�ҵ�����ֵ�������³�ʼ��
	{
		memset((void*) &g_stPCnt, 0, sizeof(tParticleCnt));
		g_stPCnt.number = 0xFFFFFFFF;	//���Ϊ����Ա�����һ��д�뵽0
	}	
}

/************************************************************
*	�������ƣ�	FRAM_Read
*	�������ܣ�	��FRAM��ȡ����
*	��ڲ�����	pData   - ���ݴ洢�ռ�
*               iAddr   - ���ݵ�ַ
*               iSize   - ��������
*				bCrc	- �Ƿ����CRCУ��
*	���ز�����	���ʳ�ʱ������ERROR�����򷵻�SUCCESS
*	˵����      
************************************************************/
ErrStatus FRAM_Read(uint8_t *pData, uint16_t iAddr, uint16_t iSize, uint8_t bCrc)
{
	ErrStatus iStatus;
	uint8_t cmd[3];
	cmd[0] = FRAM_READ;
	cmd[1] = (uint8_t) (iAddr>>8);
	cmd[2] = (uint8_t) iAddr;
	
	FRAM_CSLow();
    
    iStatus = FRAM_Transmit( cmd, 3, FRAM_TIMEOUT_VALUE);
    if (ERROR != iStatus)
	{
		if(bCrc)
			iStatus = FRAM_ReceiveCrc( pData, iSize, FRAM_TIMEOUT_VALUE);
		else
			iStatus = FRAM_Receive( pData, iSize, FRAM_TIMEOUT_VALUE);
	}
    
    FRAM_CSHigh();
    return iStatus;
}

/************************************************************
*	�������ƣ�	FRAM_WriteEn
*	�������ܣ�	ʹ��FRAMд
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_WriteEn(void)
{
	uint8_t cmd;
	int i;
	cmd = FRAM_WREN;
	
	FRAM_CSLow();
	FRAM_Transmit(&cmd, 1, FRAM_TIMEOUT_VALUE);
	FRAM_CSHigh();
	
	for (i=0; i<16; i++);	//At least 60ns;
}

/************************************************************
*	�������ƣ�	FRAM_Write
*	�������ܣ�	��FRAMд������
*	��ڲ�����	pData   - ���ݴ洢�ռ�
*               iAddr   - ���ݵ�ַ
*               iSize   - ��������
*				bCrc	- �Ƿ���CRCУ���ֽ�
*	���ز�����	���ʳ�ʱ������ERROR�����򷵻�SUCCESS
*	˵����      
************************************************************/
ErrStatus FRAM_Write(uint8_t *pData, uint16_t iAddr, uint16_t iSize, uint8_t bCrc)
{
	ErrStatus iStatus;
	uint8_t cmd[3];
	
	FRAM_WriteEn();
	
	cmd[0] = FRAM_WRITE;
	cmd[1] = (uint8_t) (iAddr>>8);
	cmd[2] = (uint8_t) iAddr;	
	
	FRAM_CSLow();	
	iStatus = FRAM_Transmit(cmd, 3, FRAM_TIMEOUT_VALUE); 
    if( ERROR != iStatus)
	{
		if(bCrc)
			iStatus = FRAM_TransmitCrc(pData, iSize, FRAM_TIMEOUT_VALUE);
		else
			iStatus = FRAM_Transmit(pData, iSize, FRAM_TIMEOUT_VALUE);
	}
	
	FRAM_CSHigh();
	return iStatus;
}

/************************************************************
*	�������ƣ�	FRAM_GetStatus
*	�������ܣ�	��ȡFRAM״̬�Ĵ���
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
uint8_t FRAM_GetStatus(void)
{
	uint8_t iStatus;
	iStatus = FRAM_RDSR;
	
	FRAM_CSLow();
	FRAM_Transmit(&iStatus, 1, FRAM_TIMEOUT_VALUE); 
	FRAM_Receive(&iStatus, 1, FRAM_TIMEOUT_VALUE);
	FRAM_CSHigh();
	
	return iStatus;	
}

/************************************************************
*	�������ƣ�	FRAM_SetStatus
*	�������ܣ�	����FRAM״̬�Ĵ���
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_SetStatus(uint8_t iStatus)
{
	uint8_t cmd;

	FRAM_WriteEn();
	
	cmd = FRAM_WRSR;
	FRAM_CSLow();
	FRAM_Transmit(&cmd, 1, FRAM_TIMEOUT_VALUE);
	FRAM_Transmit(&iStatus, 1, FRAM_TIMEOUT_VALUE);
	FRAM_CSHigh();
}

/************************************************************
*	�������ƣ�	FRAM_ProtectHalf
*	�������ܣ�	������1/2�Ĵ���
*	��ڲ�����	isProtect	- �Ƿ���б�������ȡ��������
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_ProtectHalf(uint8_t isProtect)
{
	uint8_t iStatus;
	if(isProtect)
		iStatus = FRAM_BP_HALF;
	else
		iStatus = FRAM_BP_NONE;
	FRAM_SetStatus(iStatus);
}

/************************************************************
*	�������ƣ�	FRAM_UpdateCalInfo
*	�������ܣ�	�Դ��ڱ������ı궨��Ϣ���и��£������ؽ��жԱ�
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
ErrStatus FRAM_UpdateCalInfo(void)
{
	ErrStatus iStatus=SUCCESS;
	tCalInfo stPCal;
	
	g_stPCal.sum = FRAM_CalcSum((uint8_t *) &g_stPCal, sizeof(tCalInfo)-2);	//����У���
	
	FRAM_ProtectHalf(0);	//ȡ���߰뱣��
	iStatus = FRAM_Write((uint8_t *) &g_stPCal, FRAM_ADDR_CALI, sizeof(tCalInfo), 1);
	FRAM_ProtectHalf(1);	//ʹ�ܸ߰뱣��
	
	if(iStatus == ERROR)
		return ERROR;
	
	if(ERROR == FRAM_Read((uint8_t *) &stPCal, FRAM_ADDR_CALI, sizeof(tCalInfo), 1))
		return ERROR;
	
	if(memcmp((void*) &g_stPCal, (void *) &stPCal, sizeof(tCalInfo)) != 0)	//���ݲ�һ�� 
		return ERROR;
	return SUCCESS;
}

/************************************************************
*	�������ƣ�	FRAM_UpdateCntInfo
*	�������ܣ�	�Լ�����Ϣ���и���
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
ErrStatus FRAM_UpdateCntInfo(void)
{
	uint16_t addr;	
	
	g_stPCnt.number++;
	addr = FRAM_ADDR_CNT+FRAM_CNT_SIZE*(g_stPCnt.number&0x3);	
	g_stPCnt.sum = FRAM_CalcSum((uint8_t *) &g_stPCnt, sizeof(tParticleCnt)-2);	//����У���
	
	return FRAM_Write((uint8_t *) &g_stPCnt, addr, sizeof(tParticleCnt), 1);
}

void FRAM_UpdateCorrectInfo(void)
{
	uint16_t addr;
	int i;
//	ErrStatus iStatus = SUCCESS;
	g_stCorrect.sum = FRAM_CalcSum((uint8_t *) &g_stCorrect, sizeof(tCorrectInfo)-2);	//����У���
	addr = FRAM_ADDR_CORRECT;
	for(i=0;i<4;i++)
	{
//		if(ERROR == FRAM_Write((uint8_t *) &g_stPCnt, addr, sizeof(tParticleCnt), 1))
//			iStatus = ERROR;
		FRAM_Write((uint8_t *) &g_stPCnt, addr, sizeof(tParticleCnt), 1);
		addr += FRAM_CORRECT_SIZE;
	}
}

#define FRAM_CHECK_DATA		(0xAA555AA5)
/************************************************************
*	�������ƣ�	FRAM_TestAll
*	�������ܣ�	��FRAMȫ����ж�д����Ƿ���ȷ
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_TestAll(void)
{
	uint16_t i;
	uint16_t iErrCnt, iTimeoutCnt;
	uint32_t iData;
	uint32_t iDataCheck;
    char ch;
    
	cprintf("\nTest FM25L16 All space Access.\n");
	cprintf("WARNING: The data stored will be lost.\n");
    cprintf("Do you continue?(Y/N)\n");
    ch = CS_Getch();
    if(ch != 'Y' && ch != 'y')
    {
        cprintf("Exit Fram Test\n");
        return;
    }
    
	cprintf("Test Fiexed data\n");
    iTimeoutCnt = 0;
	iData = FRAM_CHECK_DATA;
	for(i=0; i<FRAM_SIZE/4; i++)
		if(ERROR == FRAM_Write((uint8_t *)&iData, i<<2, 4, 0)) 
            iTimeoutCnt++;
	
	cprintf(" Write Complete! Check Data...\n");
	iErrCnt = 0;
	for(i=0; i<FRAM_SIZE/4; i++)
	{
		iData = 0;
		if(ERROR == FRAM_Read((uint8_t *)&iData, i<<2, 4, 0))
            iTimeoutCnt++;
		if(iData != FRAM_CHECK_DATA)
			iErrCnt++;
	}
	if(iErrCnt > 0 || iTimeoutCnt > 0)
		cprintf(" [ERROR] Read Error: %d, Time out: %d\n", iErrCnt, iTimeoutCnt);
	else
		cprintf(" [OK] Check Fixed data\n");
	
	cprintf("Test Variable Data\n");
    iTimeoutCnt = 0;
	for(i=0; i<FRAM_SIZE/4; i++)
	{
		iData = ((~i)<<9 | i)&0xFFFF;
		iData = ((~iData)<<16) | iData;
		if( ERROR == FRAM_Write((uint8_t *)&iData, i<<2, 4, 0))
            iTimeoutCnt++;
	}
	
	cprintf(" Write Complete! Check Data...\n");
	iErrCnt = 0;
	for(i=0; i<FRAM_SIZE/4; i++)
	{
		iDataCheck = ((~i)<<9 | i)&0xFFFF;
		iDataCheck = ((~iDataCheck)<<16) | iDataCheck;
		if(ERROR == FRAM_Read((uint8_t *)&iData, i<<2, 4, 0))
            iTimeoutCnt++;
		if(iData != iDataCheck)
			iErrCnt++;
	}
	if(iErrCnt > 0 || iTimeoutCnt > 0)
		cprintf(" [Error] Read Error: %d, Time out: %d\n", iErrCnt, iTimeoutCnt);
	else
		cprintf(" [OK] Check Variable data\n");
	
	cprintf("Clear All Data of FM25L16 to zero?(Y/N)(Default: N)\n");
    ch = CS_Getch();
    if(ch == 'Y' || ch == 'y')
    {
		iData = 0;
		for(i=0; i<FRAM_SIZE/4; i++)
			FRAM_Write((uint8_t *)&iData, i<<2, 4, 0); 
		cprintf("All Data Has been Cleared\n");
    }	
	
	cprintf("\nTest FM25L16 End\n");	
}

/************************************************************
*	�������ƣ�	FRAM_DisplayAll
*	�������ܣ�	��ʾFRAM�ڲ�������������
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_DisplayAll(void)
{
    uint16_t i;
    uint32_t iData;
    
    cprintf("\nRead and Display all FRAM Datas(HEX). Total: 0x%X.", FRAM_SIZE);
    for(i=0; i<FRAM_SIZE/4; i++)
    {
        if( (i&0x7) == 0) //ÿ8�ֻ�һ��
            cprintf("\n ");
        FRAM_Read((uint8_t *)&iData, i<<2, 4, 0);
        cprintf(" %08X", iData);
    }
    cprintf("\nEnd of FRAM Read\n");
}

/************************************************************
*	�������ƣ�	FRAM_Modify
*	�������ܣ�	�޸�FRAM�ڲ�ָ����ַ������
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_Modify(void)
{
    uint16_t iAddr = 0;
    uint16_t iSize = 4;
    uint32_t i32Data = 0;
    uint32_t i32DataOld = 0;
    uint8_t  *pData;
    int i;     
    char ch;
    
    cprintf("\nChange the Content of FRAM.\n");
    
    while(1)
    {
        cprintf("Input the Address for Modify(Range:0~0x%X)(Default:0x%X): ", FRAM_SIZE, iAddr);
        if( CS_GetIntNum(&i))
		{
            if(i>=0 && i<=FRAM_SIZE)
                iAddr = (uint16_t) i;
        }        
        cprintf("  Set Address As: 0x%X\n", iAddr);
        
        cprintf("Input the Data(1~4Bytes)(Default:0x%X): ", i32Data);
        if( CS_GetIntNum(&i))
            i32Data = (uint32_t) i;
        cprintf("  Set Data As: 0x%X\n", i32Data);
        
        cprintf("Input the Data length(Range:1~4)(Default:%d): ", iSize);
        if( CS_GetIntNum(&i))
		{
            if(i>=1 && i<=4)
                iSize = (uint16_t) i;
            if( (iAddr + iSize-1) > FRAM_SIZE)
                iSize = FRAM_SIZE-iAddr+1;
        }
        cprintf("  Set length As: %d\n", iSize);
        
        pData = (uint8_t *)&i32DataOld;
        if(FRAM_Read( pData, iAddr, iSize, 0))
        {            
            for( i = iSize; i<4; i++)   //��δ�õ����ֽ�����
                pData[i] = 0;            
            cprintf("  The Old Data is: 0x%X\n", i32DataOld);
        }
        else
        {
            cprintf("  Read FRAM Error.\n");
        }
        
        cprintf("Modify the data with: 0x%X?(Y/N)\n", i32Data);
        ch = CS_Getch();
        if(ch=='Y' || ch=='y')
        {
            if(FRAM_Write((uint8_t *)&i32Data, iAddr, iSize, 0))
                cprintf("  Write Success\n");
            else
                cprintf("  Write Fail\n");
        }
		
		cprintf("Modify Others?(Y/N)(Default: N)\n");
		ch = CS_Getch();
		if(ch!='Y' && ch!='y')
			break;
    }
    cprintf("Exit the FRAM Modify.\n");
}


/************************************************************
*	�������ƣ�	FRAM_PrintCalInfo
*	�������ܣ�	��ӡ��ǰ�ı궨��Ϣ
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_PrintCalInfo(void)
{
	char *pChar;
	int i;
	uint8_t u8Tmp;
	
	pChar = (char *)&(g_stPCal.magic);
	cprintf("   Magic Word:    %c%c%c%c\n", pChar[3], pChar[2], pChar[1], pChar[0]);
	cprintf("   Product SN:    WRD%08d%08d\n", g_stPCal.productSnH, g_stPCal.productSnL);
	cprintf("   Main PCBA SN:  MBO%04d%08d\n", g_stPCal.mainSnH, g_stPCal.mainSnL);
	cprintf("   COM PCBA SN:   COM%04d%08d\n", g_stPCal.comSnH, g_stPCal.comSnL);
	cprintf("   Power PCBA SN: PWO%04d%08d\n", g_stPCal.powerSnH, g_stPCal.powerSnL);
	cprintf("   Cali Date:     %s", ctime(&(g_stPCal.date))); 
	cprintf("   Cu Amplitude:  %.1f, %.1f, %.1f, %.1f, %.1f\n", __sqrtf(g_stPCal.cu_acal[0]), __sqrtf(g_stPCal.cu_acal[1]),  __sqrtf(g_stPCal.cu_acal[2]), __sqrtf(g_stPCal.cu_acal[3]), __sqrtf(g_stPCal.cu_acal[4]));
	cprintf("   Fe Amplitude:  %.1f, %.1f, %.1f, %.1f, %.1f\n", __sqrtf(g_stPCal.fe_acal[0]), __sqrtf(g_stPCal.fe_acal[1]),  __sqrtf(g_stPCal.fe_acal[2]), __sqrtf(g_stPCal.fe_acal[3]), __sqrtf(g_stPCal.fe_acal[4]));
	cprintf("   Cu Peak1 Pha:  %d, %d, %d, %d, %d\n", (int) (g_stPCal.cu_pcal1[0]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal1[1]*COEF_TANSCALE2ANGLE), 
					(int) (g_stPCal.cu_pcal1[2]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal1[3]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal1[4]*COEF_TANSCALE2ANGLE));
	cprintf("   Cu Peak2 Pha:  %d, %d, %d, %d, %d\n", (int) (g_stPCal.cu_pcal2[0]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal2[1]*COEF_TANSCALE2ANGLE),
					(int) (g_stPCal.cu_pcal2[2]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal2[3]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.cu_pcal2[4]*COEF_TANSCALE2ANGLE));
	cprintf("   Fe Peak1 Pha:  %d, %d, %d, %d, %d\n", (int) (g_stPCal.fe_pcal1[0]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal1[1]*COEF_TANSCALE2ANGLE), 
					(int) (g_stPCal.fe_pcal1[2]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal1[3]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal1[4]*COEF_TANSCALE2ANGLE));
	cprintf("   Fe Peak2 Pha:  %d, %d, %d, %d, %d\n", (int) (g_stPCal.fe_pcal2[0]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal2[1]*COEF_TANSCALE2ANGLE), 
					(int) (g_stPCal.fe_pcal2[2]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal2[3]*COEF_TANSCALE2ANGLE), (int) (g_stPCal.fe_pcal2[4]*COEF_TANSCALE2ANGLE));
	cprintf("   Store Version: V%d.%d.%d\n", g_stPCal.ver>>11, (g_stPCal.ver>>6)&0x1F, g_stPCal.ver&0x3F);
	pChar = (char *)&g_stPCal;
	u8Tmp = 0;
	for(i=0;i<sizeof(tCalInfo)-2;i++)
		u8Tmp += (uint8_t) *pChar++;
	cprintf("   Sum Check:     0x%02X[%s]\n", g_stPCal.sum, (u8Tmp==g_stPCal.sum)?"OK":"FAIL");
	cprintf("   CRC Check:     0x%02X[OK]\n", g_stPCal.crc);
}

/************************************************************
*	�������ƣ�	FRAM_SetPhaseCalData
*	�������ܣ�	����0~1023�ķ�Χ���ñ궨��λ
*	��ڲ�����	pPhaseCal	- �����õı궨��λ(0~1023)
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_SetPhaseCalData(int16_t *pPhaseCal)
{
	int iData;
	cprintf("   Old Phase: %d\n", (int) ((*pPhaseCal)*COEF_TANSCALE2ANGLE));
	cprintf("   New(Range:[0,360)): ");
	if(ERROR != CS_GetIntNum(&iData))
	{		
		iData = (int) (iData/COEF_TANSCALE2ANGLE+0.5f);
		while(iData<0)					//<0
			iData += ANGLE_PERIOD;
		while(iData>=ANGLE_PERIOD)		//>=1024
			iData -= ANGLE_PERIOD;
		*pPhaseCal = (int16_t) iData;
	}
	cprintf("   Set As: %d\n", (int) ((*pPhaseCal)*COEF_TANSCALE2ANGLE+0.5f));
}	

/************************************************************
*	�������ƣ�	FRAM_SetCalInfo
*	�������ܣ�	�ֶ����ñ궨��Ϣ
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_SetCalInfo(void)
{
	char str[32];
	char ch;
	int i;
	float fData;
	
	cprintf("Start Calibration Info Setting\n");
	if( (ERROR == FRAM_Read((uint8_t *) &g_stPCal, FRAM_ADDR_CALI, sizeof(tCalInfo), 1)) || (g_stPCal.magic != ('W'<<24 | 'R'<<16 | 'D'<<8 | 'O')))
	{
		cprintf("  [ERROR] Read Calibration Information\n");
		//��ʼ��
		g_stPCal.magic = 'W'<<24 | 'R'<<16 | 'D'<<8 | 'O';
		g_stPCal.ver   = 0x1<<6;	//V0.1.0
		g_stPCal.productSnH  = 91010022;
		g_stPCal.productSnL  = 10000000;
		g_stPCal.mainSnH     = 0122;
		g_stPCal.mainSnL	 = 10000000;
		g_stPCal.comSnH		 = 0122;
		g_stPCal.comSnL		 = 10000000;
		g_stPCal.powerSnH	 = 0122;
		g_stPCal.powerSnL	 = 10000000;
		//time(&(g_stPCal.date));
		memset((void*) &(g_stPCal.cu_acal[0]), 0xFF, 80);	//�궨������Ϊ���FF
		g_stPCal.sum = 0;
		g_stPCal.crc = 0;
	}
	else
		cprintf("  [OK] Calibration Read OK\n");
	
	cprintf("\nThe Old Calibration Information\n");
	FRAM_PrintCalInfo();	
	
	cprintf("Continue to Revise the Setting?(Y/N)\n");
	ch = CS_Getch();
	if(ch!='Y' && ch!='y')
	{
		cprintf(" Dicard Revise!\n");
		return;
	}
	
	cprintf(" Old Product SN:  WRD%08d%08d\n", g_stPCal.productSnH, g_stPCal.productSnL);
	cprintf("  New Product SN: ");
	CS_GetString(str,32);
	if(strlen(str) == 19)
		sscanf(str, "WRD%08d%08d", &g_stPCal.productSnH, &g_stPCal.productSnL);
	cprintf("  Set As: WRD%08d%08d\n", g_stPCal.productSnH, g_stPCal.productSnL);
	
	cprintf(" Old Main PCBA SN:  MBO%04d%08d\n", g_stPCal.mainSnH, g_stPCal.mainSnL);
	cprintf("  New Main PCBA SN: ");
	CS_GetString(str,32);
	if(strlen(str) == 15)
		sscanf(str, "MBO%04d%08d", &g_stPCal.mainSnH, &g_stPCal.mainSnL);
	cprintf("  Set As: MBO%04d%08d\n", g_stPCal.mainSnH, g_stPCal.mainSnL);
	
	cprintf(" Old COM PCBA SN:  COM%04d%08d\n", g_stPCal.comSnH, g_stPCal.comSnL);
	cprintf("  New COM PCBA SN: ");
	CS_GetString(str,32);
	if(strlen(str) == 15)
		sscanf(str, "COM%04d%08d", &g_stPCal.comSnH, &g_stPCal.comSnL);
	cprintf("  Set As:  COM%04d%08d\n", g_stPCal.comSnH, g_stPCal.comSnL);
	
	cprintf(" Old Power PCBA SN:  PWO%04d%08d\n", g_stPCal.powerSnH, g_stPCal.powerSnL);
	cprintf("  New Power PCBA SN: ");
	CS_GetString(str,32);
	if(strlen(str) == 15)
		sscanf(str, "PWO%04d%08d", &g_stPCal.powerSnH, &g_stPCal.powerSnL);
	cprintf("  Set As:  PWO%04d%08d\n", g_stPCal.powerSnH, g_stPCal.powerSnL);
	
	cprintf(" Old Cali Date: %s\n", ctime(&(g_stPCal.date))); 
	cprintf("  Use Current Time?(Y/N)\n");
	ch = CS_Getch();
	if(ch=='Y' || ch=='y')
		time(&(g_stPCal.date));
	cprintf("  Set As: %s\n", ctime(&(g_stPCal.date))); 
	
	cprintf(" Set Cu Amplitude\n");
	for(i=0; i<5; i++)
	{
		cprintf("  Level %d\n", i+1);
		cprintf("   Old: %.1f\n", (double) __sqrtf(g_stPCal.cu_acal[i]));
		cprintf("   New: ");
		if(ERROR != CS_GetFloatNum(&fData))
			g_stPCal.cu_acal[i] = (uint32_t) (fData*fData+0.5f);
		cprintf("   Set As: %.1f\n",  (double) __sqrtf(g_stPCal.cu_acal[i]));		
	}
	
	cprintf(" Set Fe Amplitude\n");
	for(i=0; i<5; i++)
	{
		cprintf("  Level %d\n", i+1);
		cprintf("   Old: %.1f\n", (double) __sqrtf(g_stPCal.fe_acal[i]));
		cprintf("   New: ");
		if(ERROR != CS_GetFloatNum(&fData))
			g_stPCal.fe_acal[i] = (uint32_t) (fData*fData+0.5f);
		cprintf("   Set As: %.1f\n",  (double) __sqrtf(g_stPCal.fe_acal[i]));		
	}
	
	
	cprintf(" Set Cu Phase\n");
	for(i=0; i<5; i++)
	{
		cprintf("  Level %d\n", i+1);
		cprintf("   Peak1:\n");
		FRAM_SetPhaseCalData(&(g_stPCal.cu_pcal1[i]));
		cprintf("   Peak2:\n");
		FRAM_SetPhaseCalData(&(g_stPCal.cu_pcal2[i]));	
	}
	
	cprintf(" Set Fe Phase\n");
	for(i=0; i<5; i++)
	{
		cprintf("  Level %d\n", i+1);
		cprintf("   Peak1:\n");
		FRAM_SetPhaseCalData(&(g_stPCal.fe_pcal1[i]));
		cprintf("   Peak2:\n");
		FRAM_SetPhaseCalData(&(g_stPCal.fe_pcal2[i]));			
	}	
	
	g_stPCal.crc = 0;
	
	cprintf("\n------------------------------------------\n");
	FRAM_PrintCalInfo();	
	cprintf("-------------------------------------------\n");
	cprintf("Write to FRAM?(Y/N)\n");
	ch = CS_Getch();
	if(ch=='Y'||ch=='y')
	{		
		if(ERROR == FRAM_UpdateCalInfo())
			cprintf("  [ERROR] Write FRAM\n");
		else
			cprintf("  [OK] Write FRAM\n");
	}
	else
		cprintf("  Discard writing\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	FRAM_PrintCountInfo
*	�������ܣ�	��ӡFRAM�洢�ļ�����Ϣ
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_PrintCountInfo(void)
{
	char ch;
	char *pChar;
	int i;
	uint8_t u8Tmp;

	for(i=0;i<4;i++)
	{
		if(ERROR == FRAM_Read((uint8_t *) &g_stPCnt, FRAM_CNT_SIZE*i, sizeof(tParticleCnt), 1) || (g_stPCnt.number&0x3) != i)
			cprintf("  [ERROR] Read Particle Count: %d\n", i);
		else
		{
			cprintf("  [OK] Read Particle Count: %d\n", i);
			cprintf("   No. %d\n", g_stPCnt.number);
			
			cprintf("   Cu: %d, %d, %d, %d, %d\n", g_stPCnt.CuCnt[0], g_stPCnt.CuCnt[1], g_stPCnt.CuCnt[2], g_stPCnt.CuCnt[3], g_stPCnt.CuCnt[4]);
			cprintf("   Fe: %d, %d, %d, %d, %d\n", g_stPCnt.FeCnt[0], g_stPCnt.FeCnt[1], g_stPCnt.FeCnt[2], g_stPCnt.FeCnt[3], g_stPCnt.FeCnt[4]);
			cprintf("   Overflow: 0x%04X\n", g_stPCnt.overflow);
			u8Tmp = 0;
			pChar = (char *)&g_stPCnt;
			for(ch=0;ch<sizeof(tParticleCnt)-2; ch++)
				u8Tmp += (uint8_t) *pChar++;
			cprintf("   Check Sum: 0x%02X[%s]\n", g_stPCnt.sum, (u8Tmp==g_stPCnt.sum)?"OK":"FAIL");
			cprintf("   Check CRC: 0x%02X[OK]\n", g_stPCnt.crc);
		}
	}
}

/************************************************************
*	�������ƣ�	FRAM_ClearCount
*	�������ܣ�	���FRAM�еļ�������1����0����2~3�����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_ClearCount(void)
{
	char ch;
	int i;
	uint32_t u32Data;
	
	cprintf("Start Count Info Clear\n");

	cprintf(" The Old Setting��\n");
	FRAM_PrintCountInfo();

	cprintf(" Confirm to Clear Count Setting in FRAM?(Y/N)\n");
	ch = CS_Getch();
	if(ch=='Y' || ch=='y')
	{
		memset((void*) &g_stPCnt, 0, 48);
		FRAM_Write((uint8_t *) &g_stPCnt, FRAM_ADDR_CNT, sizeof(tParticleCnt), 1);	//���µ�1��
		
		u32Data = 0x0;
		for(i=0;i<FRAM_CNT_SIZE/4*3;i++)	//��2~4��ȫ�����
			FRAM_Write((uint8_t *) &u32Data, FRAM_CNT_SIZE+(i<<2), 4, 0);
		
		cprintf(" FRAM Write OK, Read back: \n");
		FRAM_PrintCountInfo();
	}
	else
		cprintf("  No Clear\n");
	CS_WaitKey();
}

void FRAM_UpdateCnt(void)
{
	char ch;
	cprintf("Current Particle Count Infomation:\n");
	cprintf("  No.    %d\n", g_stPCnt.number);	
	cprintf("  Cu: %d, %d, %d, %d, %d\n", g_stPCnt.CuCnt[0], g_stPCnt.CuCnt[1], g_stPCnt.CuCnt[2], g_stPCnt.CuCnt[3], g_stPCnt.CuCnt[4]);
	cprintf("  Fe: %d, %d, %d, %d, %d\n", g_stPCnt.FeCnt[0], g_stPCnt.FeCnt[1], g_stPCnt.FeCnt[2], g_stPCnt.FeCnt[3], g_stPCnt.FeCnt[4]);
	cprintf("  Overflow: 0x%04X\n", g_stPCnt.overflow);
	cprintf(" Update to FRAM?(Y/N)\n");
	ch = CS_Getch();
	if(ch == 'Y' || ch=='y')
	{
		if(ERROR == FRAM_UpdateCntInfo())
			cprintf(" [ERROR] Update Fail\n");
		else
		{
			cprintf(" [OK] Update Success\n");
			FRAM_PrintCountInfo();
		}
	}
	else
		cprintf(" Cancel Updating\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	FRAM_Test
*	�������ܣ�	����FRAM��������Ŀ¼
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void FRAM_Test(void)
{
    cprintf("\nTest FRAM Functions\n");
    MENU_Disp(func_fram_test);  
    cprintf("End of FRAM Function Test\n");
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

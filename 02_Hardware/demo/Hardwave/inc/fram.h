/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               Fram.h
** Latest modified Date:    2022-5-25
** Latest Version:          1.0
** Descriptions:            Used for Fram Test
**							
**
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
#ifndef __FRAM_H__
#define __FRAM_H__

/** ---	Start of head include	---------------------------------------------------------------------- **/
#define FRAM_STATUS_WPEN_MSK		(0x80)  //写保护引脚使能（保护Status寄存器写入）
#define FRAM_STATUS_BP_MSK			(0x0C)	//内部区域保护
#define FRAM_STATUS_WEL_MSK			(0x02)	//写使能状态

//Block Protect
#define FRAM_BP_NONE				(0x00)	//无区域保护
#define FRAM_BP_QUARTER				(0x04)	//高1/4保护：0x600~0x7FF
#define FRAM_BP_HALF				(0x08)	//高1/2保护：0x400~0x7FF
#define FRAM_BP_ALL					(0x0C)	//所有区域保护：0x00~0x7FF

/** ---	Start of Macro define	---------------------------------------------------------------------- **/
/* SPI Instruction */
#define FRAM_WRITE  (0x02)    //Write memory data
#define FRAM_READ   (0x03)    //Read memory data
#define FRAM_WRSR   (0x01)    //Write Status Register
#define FRAM_RDSR   (0x05)    //Read Status Register
#define FRAM_WRDI   (0x04)    //Write disable
#define FRAM_WREN   (0x06)    //Set write enable latch

/** ---	Start of Funtion define	---------------------------------------------------------------------- **/
void FRAM_Init(void);
void FRAM_Test(void);

#endif //__FRAM_H__
/** ---	End of File	---------------------------------------------------------------------------------- **/ 

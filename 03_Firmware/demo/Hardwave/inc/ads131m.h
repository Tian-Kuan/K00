/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               ads131m.h
** Latest modified Date:    2022-5-26
** Latest Version:          1.0
** Descriptions:            Used for Test analog Sample And Related functions
**							
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-26
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
#ifndef __ADS131M_H__
#define __ADS131M_H__

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include <time.h>
#include "gd32f4xx.h"

/** ---	Start of Macro define	---------------------------------------------------------------------- **/
//颗粒计数值在FRAM中保存，使用交替保存方法，在上电读取的过程中，只取CRC校验正确，且编号较大的。
//有4个存储区，互为备份
//该数据位于低半区0位起始地址，总共占用256bytes
typedef struct
{
	uint32_t number;	//当前存储的序列号，每存储一次增1，其编号也决定了其存储位置
	uint32_t CuCnt[5];	//铜5个区间的计数值
	uint32_t FeCnt[5];	//铁5个区间的计数值
	uint16_t overflow;		//溢出标志：低8bit Cu，高8bit Fe，对应位置1表示对应区间曾经发生溢出
	uint8_t sum;		//校验和，所有数值和取反，防止出现全0情况
	uint8_t crc;		//CRC校验，由SPI自动产生
}tParticleCnt;	//颗粒计数信息（48bytes），位于低半区，64bytes间隔

typedef struct
{
	float	 cpsPhase;	//补偿相位
	uint16_t cpsRes;	//补偿电阻
	int16_t  noise;		//噪声
	int16_t  dcI;		//I相DC偏移
	int16_t  dcQ;		//Q相DC偏移
	uint16_t rsvd1;
	uint8_t  sum;
	uint8_t  crc;
}tCorrectInfo;	//校准数据，16bytes+16bytes备用, 位于256byts后，分4个区存放，共占用64bytes*2，防止保存时掉电损坏，保存四次

typedef struct
{
	uint32_t amplit;	//幅度：为两峰I/Q平方和之均值
	int16_t phase1;		//一峰相位：为圆周1024等分相位：0~1023
	int16_t phase2;		//二峰相位
	uint16_t veloc;		//速度：为峰峰值间的采样点数
	uint8_t  type;		//颗粒类型:[5:4] Cu/Fe/未识别(0: 未区分), [2:0] 区间(0: 未区分)
	uint8_t  rsvd1;
	uint32_t rsvd2;
}tParticelInf;		//颗粒信息（32bytes），应在内存中按照FIFO型式保留最新的8个磨粒信息，以便上位机查验

//在现场可进行修改的参数，应放在低半区，并应该有对应的备份区。
//该标定参数位于高半区，不可现场修改
typedef struct
{
	uint32_t magic;			//用于识别的魔术字："WRDO"
	uint32_t productSnL;	//产品序列号低字（低8十进制）
	uint32_t productSnH;	//产品序列号高字（高8十进制）
	uint32_t mainSnL;		//主板序列号低字（低8十进制）
	uint32_t mainSnH;		//主板序列号高字（高4十进制）
	uint32_t comSnL;		//通讯板序列号低字（低8十进制）
	uint32_t comSnH;		//通讯板序列号高字（高4十进制）
	uint32_t powerSnL;		//电源板序列号低字（低8十进制）
	time_t date;			//标定时间
	uint32_t powerSnH;		//电源板序列号高字（高4十进制）
	uint32_t cu_acal[5];	//铜5个区间的幅度标定值;取I/Q信号的平方和；
	int16_t cu_pcal1[5];	//铜5个区间一峰相位基准（0~1023）：一般取±30°（约±86）
	int16_t cu_pcal2[5];	//铜5个区间二峰相位基准
	uint32_t fe_acal[5];	//铁5个区间的幅度标定值;
	int16_t fe_pcal1[5];	//铁5个区间一峰相位基准
	int16_t fe_pcal2[5];	//铁5个区间二峰相位基准
	uint16_t ver;			//结构体定义版本(三段式）：bit[15:11]-高段，bit[11:6]-中段，bit[5:0]-低段	，如V0.1.0
	uint8_t sum;			//校验和
	uint8_t crc;			//CRC校验，由SPI自动产生
}tCalInfo;	//标定信息：共128bytes（放在高半区，标定完成，不允许修改）


//****************************************************************************
//
// Select the device variant to use...
//
//****************************************************************************

#define CHANNEL_COUNT (2)   // ADS131M02 -> 2 Channels

#if ((CHANNEL_COUNT < 1) || (CHANNEL_COUNT > 8))
    #error Invalid channel count configured in 'ads131m.h'.
#endif



//****************************************************************************
//
// Select the desired MODE register settings...
// NOTE: These settings will be enforced and not modifiable during runtime!
//
//****************************************************************************

/* Pick one (and only one) mode to use... */
#define WORD_LENGTH_16BIT_TRUNCATED
//#define WORD_LENGTH_24BIT
//#define WORD_LENGTH_32BIT_SIGN_EXTEND
//#define WORD_LENGTH_32BIT_ZERO_PADDED

/* Enable this define statement to use the DRDY pulse format... */
#define DRDY_FMT_PULSE

/* Enable this define statement to use CRC on DIN... */
//#define ENABLE_CRC_IN

/* Select CRC type */
#define CRC_CCITT
//#define CRC_ANSI

//
// Validation
//

// Throw an error if no WORD_LENGTH mode was selected above
#if !defined WORD_LENGTH_16BIT_TRUNCATED &&  \
    !defined WORD_LENGTH_24BIT && \
    !defined WORD_LENGTH_32BIT_SIGN_EXTEND && \
    !defined WORD_LENGTH_32BIT_ZERO_PADDED
#error Must define at least one WORD_LENGTH mode
#endif

// Throw an error if none or both CRC types are selected
#if !defined CRC_CCITT && !defined CRC_ANSI
#error Must define at least one CRC type
#endif
#if defined CRC_CCITT && defined CRC_ANSI
#error Must define only one CRC type
#endif



//****************************************************************************
//
// Constants
//
//****************************************************************************

#define NUM_REGISTERS                           ((uint8_t) 64)



//****************************************************************************
//
// SPI command opcodes
//
//****************************************************************************

#define OPCODE_NULL                             ((uint16_t) 0x0000)
#define OPCODE_RESET                            ((uint16_t) 0x0011)
#define OPCODE_RREG                             ((uint16_t) 0xA000)
#define OPCODE_WREG                             ((uint16_t) 0x6000)
#define OPCODE_STANDBY                          ((uint16_t) 0x0022)
#define OPCODE_WAKEUP                           ((uint16_t) 0x0033)
#define OPCODE_LOCK                             ((uint16_t) 0x0555)
#define OPCODE_UNLOCK                           ((uint16_t) 0x0655)



//****************************************************************************
//
// Register definitions
//
//****************************************************************************

/* NOTE: Whenever possible, macro names (defined below) were derived from
 * datasheet defined names; however, updates to documentation may cause
 * mismatches between names defined here in example code from those shown
 * in the device datasheet.
 */


/* Register 0x00 (ID) definition - READ ONLY
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                    RESERVED[3:0]                  |                    CHANCNT[3:0]                   |                                               REVID[7:0]                                              |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * For ADS131M02: 0x22??
*/

    /* ID register address & default value */
    #define ID_ADDRESS                                                      ((uint8_t)  0x00)
    #define ID_DEFAULT                                                      ((uint16_t) 0x2000 | (CHANNEL_COUNT << 8))  // NOTE: May change with future device revisions!

    /* RESERVED field mask */
    #define ID_RESERVED_MASK                                                ((uint16_t) 0xF000)

    /* CHANCNT field mask & values */
    #define ID_CHANCNT_MASK                                                 ((uint16_t) 0x0F00)
    #define ID_CHANCNT_2CH                                                  ((uint16_t) 0x0002 << 8)
    #define ID_CHANCNT_4CH                                                  ((uint16_t) 0x0004 << 8)
    #define ID_CHANCNT_6CH                                                  ((uint16_t) 0x0006 << 8)
    #define ID_CHANCNT_8CH                                                  ((uint16_t) 0x0008 << 8)

    /* REVID field mask & values */
    #define ID_REVID_MASK                                                   ((uint16_t) 0x00FF)
    #define ID_REVID_REVA                                                   ((uint16_t) 0x0000 << 0)    // DEFAULT



/* Register 0x01 (STATUS) definition - READ ONLY
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |    LOCK    |  F_RESYNC  |   REG_MAP  |   CRC_ERR  |  CRC_TYPE  |    RESET   |       WLENGTH[1:0]      |    DRDY7   |    DRDY6   |    DRDY5   |    DRDY4   |    DRDY3   |    DRDY2   |    DRDY1   |    DRDY0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  NOTE 1: Bits 0 through 7 are hardware controlled. Reading these values multiple times may return different results.
 *  NOTE 2: Bits 0 through 4 are RESERVED on the ADS131M04. These bits will always read 0.
 */

    /* STATUS register address & default value */
    #define STATUS_ADDRESS                                                  ((uint8_t)  0x01)
    #define STATUS_DEFAULT                                                  ((uint16_t) 0x0500)

    /* LOCK field mask & values */
    #define STATUS_LOCK_MASK                                                ((uint16_t) 0x8000)
    #define STATUS_LOCK_UNLOCKED                                            ((uint16_t) 0x0000 << 15)   // DEFAULT
    #define STATUS_LOCK_LOCKED                                              ((uint16_t) 0x0001 << 15)

    /* F_RESYNC field mask & values */
    #define STATUS_F_RESYNC_MASK                                            ((uint16_t) 0x4000)
    #define STATUS_F_RESYNC_NO_FAULT                                        ((uint16_t) 0x0000 << 14)   // DEFAULT
    #define STATUS_F_RESYNC_FAULT                                           ((uint16_t) 0x0001 << 14)

    /* REG_MAP field mask & values */
    #define STATUS_REG_MAP_MASK                                             ((uint16_t) 0x2000)
    #define STATUS_REG_MAP_NO_CHANGE_CRC                                    ((uint16_t) 0x0000 << 13)   // DEFAULT
    #define STATUS_REG_MAP_CHANGED_CRC                                      ((uint16_t) 0x0001 << 13)

    /* CRC_ERR field mask & values */
    #define STATUS_CRC_ERR_MASK                                             ((uint16_t) 0x1000)
    #define STATUS_CRC_ERR_NO_CRC_ERROR                                     ((uint16_t) 0x0000 << 12)   // DEFAULT
    #define STATUS_CRC_ERR_INPUT_CRC_ERROR                                  ((uint16_t) 0x0001 << 12)

    /* CRC_TYPE field mask & values */
    #define STATUS_CRC_TYPE_MASK                                            ((uint16_t) 0x0800)
    #define STATUS_CRC_TYPE_16BIT_CCITT                                     ((uint16_t) 0x0000 << 11)   // DEFAULT
    #define STATUS_CRC_TYPE_16BIT_ANSI                                      ((uint16_t) 0x0001 << 11)

    /* RESET field mask & values */
    #define STATUS_RESET_MASK                                               ((uint16_t) 0x0400)
    #define STATUS_RESET_NO_RESET                                           ((uint16_t) 0x0000 << 10)
    #define STATUS_RESET_RESET_OCCURRED                                     ((uint16_t) 0x0001 << 10)   // DEFAULT

    /* WLENGTH field mask & values */
    #define STATUS_WLENGTH_MASK                                             ((uint16_t) 0x0300)
    #define STATUS_WLENGTH_16BIT                                            ((uint16_t) 0x0000 << 8)
    #define STATUS_WLENGTH_24BIT                                            ((uint16_t) 0x0001 << 8)    // DEFAULT
    #define STATUS_WLENGTH_32BIT_LSB_ZEROES                                 ((uint16_t) 0x0002 << 8)
    #define STATUS_WLENGTH_32BIT_MSB_SIGN_EXT                               ((uint16_t) 0x0003 << 8)

#if (CHANNEL_COUNT > 7)

    /* DRDY7 field mask & values */
    #define STATUS_DRDY7_MASK                                               ((uint16_t) 0x0080)
    #define STATUS_DRDY7_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 7)
    #define STATUS_DRDY7_NEW_DATA                                           ((uint16_t) 0x0001 << 7)

#endif
#if (CHANNEL_COUNT > 6)

    /* DRDY6 field mask & values */
    #define STATUS_DRDY6_MASK                                               ((uint16_t) 0x0040)
    #define STATUS_DRDY6_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 6)
    #define STATUS_DRDY6_NEW_DATA                                           ((uint16_t) 0x0001 << 6)

#endif
#if (CHANNEL_COUNT > 5)

    /* DRDY5 field mask & values */
    #define STATUS_DRDY5_MASK                                               ((uint16_t) 0x0020)
    #define STATUS_DRDY5_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 5)
    #define STATUS_DRDY5_NEW_DATA                                           ((uint16_t) 0x0001 << 5)

#endif
#if (CHANNEL_COUNT > 4)

    /* DRDY4 field mask & values */
    #define STATUS_DRDY4_MASK                                               ((uint16_t) 0x0010)
    #define STATUS_DRDY4_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 4)
    #define STATUS_DRDY4_NEW_DATA                                           ((uint16_t) 0x0001 << 4)

#endif
#if (CHANNEL_COUNT > 3)

    /* DRDY3 field mask & values */
    #define STATUS_DRDY3_MASK                                               ((uint16_t) 0x0008)
    #define STATUS_DRDY3_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 3)
    #define STATUS_DRDY3_NEW_DATA                                           ((uint16_t) 0x0001 << 3)

#endif
#if (CHANNEL_COUNT > 2)

    /* DRDY2 field mask & values */
    #define STATUS_DRDY2_MASK                                               ((uint16_t) 0x0004)
    #define STATUS_DRDY2_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 2)
    #define STATUS_DRDY2_NEW_DATA                                           ((uint16_t) 0x0001 << 2)

#endif
#if (CHANNEL_COUNT > 1)

    /* DRDY1 field mask & values */
    #define STATUS_DRDY1_MASK                                               ((uint16_t) 0x0002)
    #define STATUS_DRDY1_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 1)
    #define STATUS_DRDY1_NEW_DATA                                           ((uint16_t) 0x0001 << 1)

#endif

    /* DRDY0 field mask & values */
    #define STATUS_DRDY0_MASK                                               ((uint16_t) 0x0001)
    #define STATUS_DRDY0_NO_NEW_DATA                                        ((uint16_t) 0x0000 << 0)
    #define STATUS_DRDY0_NEW_DATA                                           ((uint16_t) 0x0001 << 0)



/* Register 0x02 (MODE) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |      RESERVED0[1:0]     | REG_CRC_EN |  RX_CRC_EN |  CRC_TYPE  |    RESET   |       WLENGTH[1:0]      |             RESERVED1[2:0]           |   TIMEOUT  |       DRDY_SEL[1:0]     |  DRDY_HiZ  |  DRDY_FMT  |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* MODE register address & default value */
    #define MODE_ADDRESS                                                    ((uint8_t)  0x02)
    #define MODE_DEFAULT                                                    ((uint16_t) 0x0410)	//默认为16bits，重启后为0x0510

    /* RESERVED0 field mask */
    #define MODE_RESERVED0_MASK                                             ((uint16_t) 0xC000)

    /* REG_CRC_EN field mask & values */
    #define MODE_REG_CRC_EN_MASK                                            ((uint16_t) 0x2000)
    #define MODE_REG_CRC_EN_DISABLED                                        ((uint16_t) 0x0000 << 13)   // DEFAULT
    #define MODE_REG_CRC_EN_ENABLED                                         ((uint16_t) 0x0001 << 13)

    /* RX_CRC_EN field mask & values */
    #define MODE_RX_CRC_EN_MASK                                             ((uint16_t) 0x1000)
    #define MODE_RX_CRC_EN_DISABLED                                         ((uint16_t) 0x0000 << 12)   // DEFAULT
    #define MODE_RX_CRC_EN_ENABLED                                          ((uint16_t) 0x0001 << 12)

    /* CRC_TYPE field mask & values */
    #define MODE_CRC_TYPE_MASK                                              ((uint16_t) 0x0800)
    #define MODE_CRC_TYPE_16BIT_CCITT                                       ((uint16_t) 0x0000 << 11)   // DEFAULT
    #define MODE_CRC_TYPE_16BIT_ANSI                                        ((uint16_t) 0x0001 << 11)

    /* RESET field mask & values */
    #define MODE_RESET_MASK                                                 ((uint16_t) 0x0400)
    #define MODE_RESET_NO_RESET                                             ((uint16_t) 0x0000 << 10)
    #define MODE_RESET_RESET_OCCURRED                                       ((uint16_t) 0x0001 << 10)   // DEFAULT

    /* WLENGTH field mask & values */
    #define MODE_WLENGTH_MASK                                               ((uint16_t) 0x0300)
    #define MODE_WLENGTH_16BIT                                              ((uint16_t) 0x0000 << 8)
    #define MODE_WLENGTH_24BIT                                              ((uint16_t) 0x0001 << 8)
    #define MODE_WLENGTH_32BIT_LSB_ZEROES                                   ((uint16_t) 0x0002 << 8)
    #define MODE_WLENGTH_32BIT_MSB_SIGN_EXT                                 ((uint16_t) 0x0003 << 8)

    /* RESERVED1 field mask */
    #define MODE_RESERVED1_MASK                                             ((uint16_t) 0x00E0)

    /* TIMEOUT field mask & values */
    #define MODE_TIMEOUT_MASK                                               ((uint16_t) 0x0010)
    #define MODE_TIMEOUT_DISABLED                                           ((uint16_t) 0x0000 << 4)
    #define MODE_TIMEOUT_ENABLED                                            ((uint16_t) 0x0001 << 4)    // DEFAULT

    /* DRDY_SEL field mask & values */
    #define MODE_DRDY_SEL_MASK                                              ((uint16_t) 0x000C)
    #define MODE_DRDY_SEL_MOST_LAGGING                                      ((uint16_t) 0x0000 << 2)    // DEFAULT
    #define MODE_DRDY_SEL_LOGIC_OR                                          ((uint16_t) 0x0001 << 2)
    #define MODE_DRDY_SEL_MOST_LEADING                                      ((uint16_t) 0x0002 << 2)    // Alternative value: ((uint16_t) 0x0003 << 2)

    /* DRDY_HiZ field mask & values */
    #define MODE_DRDY_HiZ_MASK                                              ((uint16_t) 0x0002)
    #define MODE_DRDY_HiZ_LOGIC_HIGH                                        ((uint16_t) 0x0000 << 1)    // DEFAULT
    #define MODE_DRDY_HiZ_HIGH_IMPEDANCE                                    ((uint16_t) 0x0001 << 1)

    /* DRDY_FMT field mask & values */
    #define MODE_DRDY_FMT_MASK                                              ((uint16_t) 0x0001)
    #define MODE_DRDY_FMT_LOGIC_LOW                                         ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define MODE_DRDY_FMT_NEG_PULSE_FIXED_WIDTH                             ((uint16_t) 0x0001 << 0)



/* Register 0x03 (CLOCK) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   CH7_EN   |   CH6_EN   |   CH5_EN   |   CH4_EN   |   CH3_EN   |   CH2_EN   |   CH1_EN   |   CH0_EN   |             RESERVED[2:0]            |               OSR[2:0]               |         PWR[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  NOTE 1: Bits 12 through 15 are RESERVED on the ADS131M04.
 */

    /* CLOCK register address & default value */
    #define CLOCK_ADDRESS                                                   ((uint8_t)  0x03)

    #if (CHANNEL_COUNT == 8)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0xFF0E)
    #endif
    #if (CHANNEL_COUNT == 7)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x7F0E)
    #endif
    #if (CHANNEL_COUNT == 6)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x3F0E)
    #endif
    #if (CHANNEL_COUNT == 5)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x1F0E)
    #endif
    #if (CHANNEL_COUNT == 4)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x0F0E)
    #endif
    #if (CHANNEL_COUNT == 3)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x070E)
    #endif
    #if (CHANNEL_COUNT == 2)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x030E)
    #endif
    #if (CHANNEL_COUNT == 1)
    #define CLOCK_DEFAULT                                                   ((uint16_t) 0x010E)
    #endif

#if (CHANNEL_COUNT > 7)

    /* CH7_EN field mask & values */
    #define CLOCK_CH7_EN_MASK                                               ((uint16_t) 0x8000)
    #define CLOCK_CH7_EN_DISABLED                                           ((uint16_t) 0x0000 << 15)
    #define CLOCK_CH7_EN_ENABLED                                            ((uint16_t) 0x0001 << 15)

#endif
#if (CHANNEL_COUNT > 6)

    /* CH6_EN field mask & values */
    #define CLOCK_CH6_EN_MASK                                               ((uint16_t) 0x4000)
    #define CLOCK_CH6_EN_DISABLED                                           ((uint16_t) 0x0000 << 14)
    #define CLOCK_CH6_EN_ENABLED                                            ((uint16_t) 0x0001 << 14)

#endif
#if (CHANNEL_COUNT > 5)

    /* CH5_EN field mask & values */
    #define CLOCK_CH5_EN_MASK                                               ((uint16_t) 0x2000)
    #define CLOCK_CH5_EN_DISABLED                                           ((uint16_t) 0x0000 << 13)
    #define CLOCK_CH5_EN_ENABLED                                            ((uint16_t) 0x0001 << 13)

#endif
#if (CHANNEL_COUNT > 4)

    /* CH4_EN field mask & values */
    #define CLOCK_CH4_EN_MASK                                               ((uint16_t) 0x1000)
    #define CLOCK_CH4_EN_DISABLED                                           ((uint16_t) 0x0000 << 12)
    #define CLOCK_CH4_EN_ENABLED                                            ((uint16_t) 0x0001 << 12)

#endif
#if (CHANNEL_COUNT > 3)

    /* CH3_EN field mask & values */
    #define CLOCK_CH3_EN_MASK                                               ((uint16_t) 0x0800)
    #define CLOCK_CH3_EN_DISABLED                                           ((uint16_t) 0x0000 << 11)
    #define CLOCK_CH3_EN_ENABLED                                            ((uint16_t) 0x0001 << 11)

#endif
#if (CHANNEL_COUNT > 2)

    /* CH2_EN field mask & values */
    #define CLOCK_CH2_EN_MASK                                               ((uint16_t) 0x0400)
    #define CLOCK_CH2_EN_DISABLED                                           ((uint16_t) 0x0000 << 10)
    #define CLOCK_CH2_EN_ENABLED                                            ((uint16_t) 0x0001 << 10)

#endif
#if (CHANNEL_COUNT > 1)

    /* CH1_EN field mask & values */
    #define CLOCK_CH1_EN_MASK                                               ((uint16_t) 0x0200)
    #define CLOCK_CH1_EN_DISABLED                                           ((uint16_t) 0x0000 << 9)
    #define CLOCK_CH1_EN_ENABLED                                            ((uint16_t) 0x0001 << 9)

#endif

    /* CH0_EN field mask & values */
    #define CLOCK_CH0_EN_MASK                                               ((uint16_t) 0x0100)
    #define CLOCK_CH0_EN_DISABLED                                           ((uint16_t) 0x0000 << 8)
    #define CLOCK_CH0_EN_ENABLED                                            ((uint16_t) 0x0001 << 8)    // DEFAULT

	#define CLOCK_CHALL_DISABLED											((uint16_t) 0x0000)

    /* RESERVED1 field mask */
    #define CLOCK_RESERVED_MASK                                             ((uint16_t) 0x00E0)

    /* OSR field mask & values */
    #define CLOCK_OSR_MASK                                                  ((uint16_t) 0x001C)
    #define CLOCK_OSR_128                                                   ((uint16_t) 0x0000 << 2)
    #define CLOCK_OSR_256                                                   ((uint16_t) 0x0001 << 2)
    #define CLOCK_OSR_512                                                   ((uint16_t) 0x0002 << 2)
    #define CLOCK_OSR_1024                                                  ((uint16_t) 0x0003 << 2)    // DEFAULT
    #define CLOCK_OSR_2048                                                  ((uint16_t) 0x0004 << 2)
    #define CLOCK_OSR_4096                                                  ((uint16_t) 0x0005 << 2)
    #define CLOCK_OSR_8192                                                  ((uint16_t) 0x0006 << 2)
    #define CLOCK_OSR_16384                                                 ((uint16_t) 0x0007 << 2)

    /* PWR field mask & values */
    #define CLOCK_PWR_MASK                                                  ((uint16_t) 0x0003)
    #define CLOCK_PWR_VLP                                                   ((uint16_t) 0x0000 << 0)
    #define CLOCK_PWR_LP                                                    ((uint16_t) 0x0001 << 0)
    #define CLOCK_PWR_HR                                                    ((uint16_t) 0x0002 << 0)     // DEFAULT, Alternative value: ((uint16_t) 0x0003 << 2)



/* Register 0x04 (GAIN1) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |  RESERVED0 |             PGAGAIN3[2:0]            |  RESERVED1 |             PGAGAIN2[2:0]            |  RESERVED2 |             PGAGAIN1[2:0]            |  RESERVED3 |             PGAGAIN0[2:0]            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* GAIN1 register address & default value */
    #define GAIN1_ADDRESS                                                   ((uint8_t)  0x04)
    #define GAIN1_DEFAULT                                                   ((uint16_t) 0x0000)

    /* RESERVED0 field mask & values */
    #define GAIN1_RESERVED0_MASK                                            ((uint16_t) 0x8000)

    /* PGAGAIN3 field mask & values */
    #define GAIN1_PGAGAIN3_MASK                                             ((uint16_t) 0x7000)
    #define GAIN1_PGAGAIN3_1                                                ((uint16_t) 0x0000 << 12)    // DEFAULT
    #define GAIN1_PGAGAIN3_2                                                ((uint16_t) 0x0001 << 12)
    #define GAIN1_PGAGAIN3_4                                                ((uint16_t) 0x0002 << 12)
    #define GAIN1_PGAGAIN3_8                                                ((uint16_t) 0x0003 << 12)
    #define GAIN1_PGAGAIN3_16                                               ((uint16_t) 0x0004 << 12)
    #define GAIN1_PGAGAIN3_32                                               ((uint16_t) 0x0005 << 12)
    #define GAIN1_PGAGAIN3_64                                               ((uint16_t) 0x0006 << 12)
    #define GAIN1_PGAGAIN3_128                                              ((uint16_t) 0x0007 << 12)

    /* RESERVED1 field mask & values */
    #define GAIN1_RESERVED1_MASK                                            ((uint16_t) 0x0800)

    /* PGAGAIN2 field mask & values */
    #define GAIN1_PGAGAIN2_MASK                                             ((uint16_t) 0x0700)
    #define GAIN1_PGAGAIN2_1                                                ((uint16_t) 0x0000 << 8)    // DEFAULT
    #define GAIN1_PGAGAIN2_2                                                ((uint16_t) 0x0001 << 8)
    #define GAIN1_PGAGAIN2_4                                                ((uint16_t) 0x0002 << 8)
    #define GAIN1_PGAGAIN2_8                                                ((uint16_t) 0x0003 << 8)
    #define GAIN1_PGAGAIN2_16                                               ((uint16_t) 0x0004 << 8)
    #define GAIN1_PGAGAIN2_32                                               ((uint16_t) 0x0005 << 8)
    #define GAIN1_PGAGAIN2_64                                               ((uint16_t) 0x0006 << 8)
    #define GAIN1_PGAGAIN2_128                                              ((uint16_t) 0x0007 << 8)

    /* RESERVED2 field mask & values */
    #define GAIN1_RESERVED2_MASK                                            ((uint16_t) 0x0080)

    /* PGAGAIN1 field mask & values */
    #define GAIN1_PGAGAIN1_MASK                                             ((uint16_t) 0x0070)
    #define GAIN1_PGAGAIN1_1                                                ((uint16_t) 0x0000 << 4)    // DEFAULT
    #define GAIN1_PGAGAIN1_2                                                ((uint16_t) 0x0001 << 4)
    #define GAIN1_PGAGAIN1_4                                                ((uint16_t) 0x0002 << 4)
    #define GAIN1_PGAGAIN1_8                                                ((uint16_t) 0x0003 << 4)
    #define GAIN1_PGAGAIN1_16                                               ((uint16_t) 0x0004 << 4)
    #define GAIN1_PGAGAIN1_32                                               ((uint16_t) 0x0005 << 4)
    #define GAIN1_PGAGAIN1_64                                               ((uint16_t) 0x0006 << 4)
    #define GAIN1_PGAGAIN1_128                                              ((uint16_t) 0x0007 << 4)

    /* RESERVED3 field mask & values */
    #define GAIN1_RESERVED3_MASK                                            ((uint16_t) 0x0008)

    /* PGAGAIN0 field mask & values */
    #define GAIN1_PGAGAIN0_MASK                                             ((uint16_t) 0x0007)
    #define GAIN1_PGAGAIN0_1                                                ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define GAIN1_PGAGAIN0_2                                                ((uint16_t) 0x0001 << 0)
    #define GAIN1_PGAGAIN0_4                                                ((uint16_t) 0x0002 << 0)
    #define GAIN1_PGAGAIN0_8                                                ((uint16_t) 0x0003 << 0)
    #define GAIN1_PGAGAIN0_16                                               ((uint16_t) 0x0004 << 0)
    #define GAIN1_PGAGAIN0_32                                               ((uint16_t) 0x0005 << 0)
    #define GAIN1_PGAGAIN0_64                                               ((uint16_t) 0x0006 << 0)
    #define GAIN1_PGAGAIN0_128                                              ((uint16_t) 0x0007 << 0)





/* Register 0x05 (GAIN2) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |  RESERVED0 |             PGAGAIN7[2:0]            |  RESERVED1 |             PGAGAIN6[2:0]            |  RESERVED2 |             PGAGAIN5[2:0]            |  RESERVED3 |             PGAGAIN4[2:0]            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * NOTE 1: This register is RESERVED on the ADS131M04. Only write 0x0000 to this register.
 */

    /* GAIN2 register address & default value */
    #define GAIN2_ADDRESS                                                   ((uint8_t)  0x05)
    #define GAIN2_DEFAULT                                                   ((uint16_t) 0x0000)

#if (CHANNEL_COUNT > 4)

    /* RESERVED0 field mask & values */
    #define GAIN2_RESERVED0_MASK                                            ((uint16_t) 0x8000)

    /* PGAGAIN7 field mask & values */
    #define GAIN2_PGAGAIN7_MASK                                             ((uint16_t) 0x7000)
    #define GAIN2_PGAGAIN7_1                                                ((uint16_t) 0x0000 << 12)   // DEFAULT
    #define GAIN2_PGAGAIN7_2                                                ((uint16_t) 0x0001 << 12)
    #define GAIN2_PGAGAIN7_4                                                ((uint16_t) 0x0002 << 12)
    #define GAIN2_PGAGAIN7_8                                                ((uint16_t) 0x0003 << 12)
    #define GAIN2_PGAGAIN7_16                                               ((uint16_t) 0x0004 << 12)
    #define GAIN2_PGAGAIN7_32                                               ((uint16_t) 0x0005 << 12)
    #define GAIN2_PGAGAIN7_64                                               ((uint16_t) 0x0006 << 12)
    #define GAIN2_PGAGAIN7_128                                              ((uint16_t) 0x0007 << 12)

    /* RESERVED1 field mask & values */
    #define GAIN2_RESERVED1_MASK                                            ((uint16_t) 0x0800)

    /* PGAGAIN6 field mask & values */
    #define GAIN2_PGAGAIN6_MASK                                             ((uint16_t) 0x0700)
    #define GAIN2_PGAGAIN6_1                                                ((uint16_t) 0x0000 << 8)    // DEFAULT
    #define GAIN2_PGAGAIN6_2                                                ((uint16_t) 0x0001 << 8)
    #define GAIN2_PGAGAIN6_4                                                ((uint16_t) 0x0002 << 8)
    #define GAIN2_PGAGAIN6_8                                                ((uint16_t) 0x0003 << 8)
    #define GAIN2_PGAGAIN6_16                                               ((uint16_t) 0x0004 << 8)
    #define GAIN2_PGAGAIN6_32                                               ((uint16_t) 0x0005 << 8)
    #define GAIN2_PGAGAIN6_64                                               ((uint16_t) 0x0006 << 8)
    #define GAIN2_PGAGAIN6_128                                              ((uint16_t) 0x0007 << 8)

    /* RESERVED2 field mask & values */
    #define GAIN2_RESERVED2_MASK                                            ((uint16_t) 0x0080)

    /* PGAGAIN5 field mask & values */
    #define GAIN2_PGAGAIN5_MASK                                             ((uint16_t) 0x0070)
    #define GAIN2_PGAGAIN5_1                                                ((uint16_t) 0x0000 << 4)    // DEFAULT
    #define GAIN2_PGAGAIN5_2                                                ((uint16_t) 0x0001 << 4)
    #define GAIN2_PGAGAIN5_4                                                ((uint16_t) 0x0002 << 4)
    #define GAIN2_PGAGAIN5_8                                                ((uint16_t) 0x0003 << 4)
    #define GAIN2_PGAGAIN5_16                                               ((uint16_t) 0x0004 << 4)
    #define GAIN2_PGAGAIN5_32                                               ((uint16_t) 0x0005 << 4)
    #define GAIN2_PGAGAIN5_64                                               ((uint16_t) 0x0006 << 4)
    #define GAIN2_PGAGAIN5_128                                              ((uint16_t) 0x0007 << 4)

    /* RESERVED3 field mask & values */
    #define GAIN2_RESERVED3_MASK                                            ((uint16_t) 0x0008)

    /* PGAGAIN4 field mask & values */
    #define GAIN2_PGAGAIN4_MASK                                             ((uint16_t) 0x0007)
    #define GAIN2_PGAGAIN4_1                                                ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define GAIN2_PGAGAIN4_2                                                ((uint16_t) 0x0001 << 0)
    #define GAIN2_PGAGAIN4_4                                                ((uint16_t) 0x0002 << 0)
    #define GAIN2_PGAGAIN4_8                                                ((uint16_t) 0x0003 << 0)
    #define GAIN2_PGAGAIN4_16                                               ((uint16_t) 0x0004 << 0)
    #define GAIN2_PGAGAIN4_32                                               ((uint16_t) 0x0005 << 0)
    #define GAIN2_PGAGAIN4_64                                               ((uint16_t) 0x0006 << 0)
    #define GAIN2_PGAGAIN4_128                                              ((uint16_t) 0x0007 << 0)

#endif



/* Register 0x06 (CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |            RESERVED0[2:0]            |                    GC_DLY[3:0]                    |    GC_EN   |  CD_ALLCH  |              CD_NUM[2:0]             |              CD_LEN[2:0]             |    CD_EN   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CFG register address & default value */
    #define CFG_ADDRESS                                                     ((uint8_t)  0x06)
    #define CFG_DEFAULT                                                     ((uint16_t) 0x0600)

    /* RESERVED0 field mask & values */
    #define CFG_RESERVED0_MASK                                              ((uint16_t) 0xE000)

    /* GC_DLY field mask & values */
    #define CFG_GC_DLY_MASK                                                 ((uint16_t) 0x1E00)
    #define CFG_GC_DLY_2                                                    ((uint16_t) 0x0000 << 9)
    #define CFG_GC_DLY_4                                                    ((uint16_t) 0x0001 << 9)
    #define CFG_GC_DLY_8                                                    ((uint16_t) 0x0002 << 9)
    #define CFG_GC_DLY_16                                                   ((uint16_t) 0x0003 << 9)    // DEFAULT
    #define CFG_GC_DLY_32                                                   ((uint16_t) 0x0004 << 9)
    #define CFG_GC_DLY_64                                                   ((uint16_t) 0x0005 << 9)
    #define CFG_GC_DLY_128                                                  ((uint16_t) 0x0006 << 9)
    #define CFG_GC_DLY_256                                                  ((uint16_t) 0x0007 << 9)
    #define CFG_GC_DLY_512                                                  ((uint16_t) 0x0008 << 9)
    #define CFG_GC_DLY_1024                                                 ((uint16_t) 0x0009 << 9)
    #define CFG_GC_DLY_2048                                                 ((uint16_t) 0x000A << 9)
    #define CFG_GC_DLY_4096                                                 ((uint16_t) 0x000B << 9)
    #define CFG_GC_DLY_8192                                                 ((uint16_t) 0x000C << 9)
    #define CFG_GC_DLY_16484                                                ((uint16_t) 0x000D << 9)
    #define CFG_GC_DLY_32768                                                ((uint16_t) 0x000E << 9)
    #define CFG_GC_DLY_65536                                                ((uint16_t) 0x000F << 9)

    /* GC_EN field mask & values */
    #define CFG_GC_EN_MASK                                                  ((uint16_t) 0x0100)
    #define CFG_GC_EN_DISABLED                                              ((uint16_t) 0x0000 << 8)    // DEFAULT
    #define CFG_GC_EN_ENABLED                                               ((uint16_t) 0x0001 << 8)

    /* CD_ALLCH field mask & values */
    #define CFG_CD_ALLCH_MASK                                               ((uint16_t) 0x0080)
    #define CFG_CD_ALLCH_ANY_CHANNEL                                        ((uint16_t) 0x0000 << 7)    // DEFAULT
    #define CFG_CD_ALLCH_ALL_CHANNELS                                       ((uint16_t) 0x0001 << 7)

    /* CD_NUM field mask & values */
    #define CFG_CD_NUM_MASK                                                 ((uint16_t) 0x0070)
    #define CFG_CD_NUM_1                                                    ((uint16_t) 0x0000 << 4)    // DEFAULT
    #define CFG_CD_NUM_2                                                    ((uint16_t) 0x0001 << 4)
    #define CFG_CD_NUM_4                                                    ((uint16_t) 0x0002 << 4)
    #define CFG_CD_NUM_8                                                    ((uint16_t) 0x0003 << 4)
    #define CFG_CD_NUM_16                                                   ((uint16_t) 0x0004 << 4)
    #define CFG_CD_NUM_32                                                   ((uint16_t) 0x0005 << 4)
    #define CFG_CD_NUM_64                                                   ((uint16_t) 0x0006 << 4)
    #define CFG_CD_NUM_128                                                  ((uint16_t) 0x0007 << 4)

    /* CD_LEN field mask & values */
    #define CFG_CD_LEN_MASK                                                 ((uint16_t) 0x000E)
    #define CFG_CD_LEN_128                                                  ((uint16_t) 0x0000 << 1)    // DEFAULT
    #define CFG_CD_LEN_256                                                  ((uint16_t) 0x0001 << 1)
    #define CFG_CD_LEN_512                                                  ((uint16_t) 0x0002 << 1)
    #define CFG_CD_LEN_768                                                  ((uint16_t) 0x0003 << 1)
    #define CFG_CD_LEN_1280                                                 ((uint16_t) 0x0004 << 1)
    #define CFG_CD_LEN_1792                                                 ((uint16_t) 0x0005 << 1)
    #define CFG_CD_LEN_2560                                                 ((uint16_t) 0x0006 << 1)
    #define CFG_CD_LEN_3584                                                 ((uint16_t) 0x0007 << 1)

    /* CD_EN field mask & values */
    #define CFG_CD_EN_MASK                                                  ((uint16_t) 0x0001)
    #define CFG_CD_EN_DISABLED                                              ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CFG_CD_EN_ENABLED                                               ((uint16_t) 0x0001 << 0)



/* Register 0x07 (THRSHLD_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                CD_TH_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* THRSHLD_MSB register address & default value */
    #define THRSHLD_MSB_ADDRESS                                             ((uint8_t)  0x07)
    #define THRSHLD_MSB_DEFAULT                                             ((uint16_t) 0x0000)

    /* CD_TH_MSB field mask & values */
    #define THRSHLD_MSB_CD_TH_MSB_MASK                                      ((uint16_t) 0xFFFF)



/* Register 0x08 (THRSHLD_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             CD_TH_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* THRSHLD_LSB register address & default value */
    #define THRSHLD_LSB_ADDRESS                                             ((uint8_t)  0x08)
    #define THRSHLD_LSB_DEFAULT                                             ((uint16_t) 0x0000)

    /* CD_TH_LSB field mask & values */
    #define THRSHLD_LSB_CD_TH_LSB_MASK                                      ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define THRSHLD_LSB_RESERVED0_MASK                                      ((uint16_t) 0x00FF)



/* Register 0x09 (CH0_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE0[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX0[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH0_CFG register address & default value */
    #define CH0_CFG_ADDRESS                                                 ((uint8_t)  0x09)
    #define CH0_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE0 field mask & values */
    #define CH0_CFG_PHASE0_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH0_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX0 field mask & values */
    #define CH0_CFG_MUX0_MASK                                               ((uint16_t) 0x0003)
    #define CH0_CFG_MUX0_AIN0P_AIN0N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH0_CFG_MUX0_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH0_CFG_MUX0_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH0_CFG_MUX0_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x0A (CH0_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL0_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH0_OCAL_MSB register address & default value */
    #define CH0_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x0A)
    #define CH0_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL0_MSB field mask & values */
    #define CH0_OCAL_MSB_OCAL0_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x0B (CH0_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL0_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH0_OCAL_LSB register address & default value */
    #define CH0_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x0B)
    #define CH0_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL0_LSB field mask & values */
    #define CH0_OCAL_LSB_OCAL0_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH0_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x0C (CH0_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL0_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH0_GCAL_MSB register address & default value */
    #define CH0_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x0C)
    #define CH0_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL0_MSB field mask & values */
    #define CH0_GCAL_MSB_GCAL0_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x0D (CH0_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL0_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH0_GCAL_LSB register address & default value */
    #define CH0_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x0D)
    #define CH0_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL0_LSB field mask & values */
    #define CH0_GCAL_LSB_GCAL0_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH0_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#if (CHANNEL_COUNT > 1)

/* Register 0x0E (CH1_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE1[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX1[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH1_CFG register address & default value */
    #define CH1_CFG_ADDRESS                                                 ((uint8_t)  0x0E)
    #define CH1_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE1 field mask & values */
    #define CH1_CFG_PHASE1_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH1_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX1 field mask & values */
    #define CH1_CFG_MUX1_MASK                                               ((uint16_t) 0x0003)
    #define CH1_CFG_MUX1_AIN1P_AIN1N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH1_CFG_MUX1_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH1_CFG_MUX1_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH1_CFG_MUX1_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x0F (CH1_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL1_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH1_OCAL_MSB register address & default value */
    #define CH1_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x0F)
    #define CH1_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL1_MSB field mask & values */
    #define CH1_OCAL_MSB_OCAL1_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x10 (CH1_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL1_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH1_OCAL_LSB register address & default value */
    #define CH1_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x10)
    #define CH1_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL1_LSB field mask & values */
    #define CH1_OCAL_LSB_OCAL1_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH1_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x11 (CH1_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL1_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH1_GCAL_MSB register address & default value */
    #define CH1_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x11)
    #define CH1_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL1_MSB field mask & values */
    #define CH1_GCAL_MSB_GCAL1_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x12 (CH1_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL1_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH1_GCAL_LSB register address & default value */
    #define CH1_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x12)
    #define CH1_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL1_LSB field mask & values */
    #define CH1_GCAL_LSB_GCAL1_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH1_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 2)

/* Register 0x13 (CH2_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE2[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX2[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH2_CFG register address & default value */
    #define CH2_CFG_ADDRESS                                                 ((uint8_t)  0x13)
    #define CH2_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE2 field mask & values */
    #define CH2_CFG_PHASE2_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH2_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX2 field mask & values */
    #define CH2_CFG_MUX2_MASK                                               ((uint16_t) 0x0003)
    #define CH2_CFG_MUX2_AIN2P_AIN2N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH2_CFG_MUX2_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH2_CFG_MUX2_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH2_CFG_MUX2_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x14 (CH2_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL2_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH2_OCAL_MSB register address & default value */
    #define CH2_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x14)
    #define CH2_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL2_MSB field mask & values */
    #define CH2_OCAL_MSB_OCAL2_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x15 (CH2_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL2_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH2_OCAL_LSB register address & default value */
    #define CH2_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x15)
    #define CH2_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL2_LSB field mask & values */
    #define CH2_OCAL_LSB_OCAL2_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH2_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x16 (CH2_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL2_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH2_GCAL_MSB register address & default value */
    #define CH2_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x16)
    #define CH2_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL2_MSB field mask & values */
    #define CH2_GCAL_MSB_GCAL2_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x17 (CH2_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL2_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH2_GCAL_LSB register address & default value */
    #define CH2_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x17)
    #define CH2_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL2_LSB field mask & values */
    #define CH2_GCAL_LSB_GCAL2_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH2_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 3)

/* Register 0x18 (CH3_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE3[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX3[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH3_CFG register address & default value */
    #define CH3_CFG_ADDRESS                                                 ((uint8_t)  0x18)
    #define CH3_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE3 field mask & values */
    #define CH3_CFG_PHASE3_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH3_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX3 field mask & values */
    #define CH3_CFG_MUX3_MASK                                               ((uint16_t) 0x0003)
    #define CH3_CFG_MUX3_AIN3P_AIN3N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH3_CFG_MUX3_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH3_CFG_MUX3_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH3_CFG_MUX3_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x19 (CH3_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL3_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH3_OCAL_MSB register address & default value */
    #define CH3_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x19)
    #define CH3_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL3_MSB field mask & values */
    #define CH3_OCAL_MSB_OCAL3_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x1A (CH3_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL3_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH3_OCAL_LSB register address & default value */
    #define CH3_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x1A)
    #define CH3_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL3_LSB field mask & values */
    #define CH3_OCAL_LSB_OCAL3_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH3_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x1B (CH3_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL3_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH3_GCAL_MSB register address & default value */
    #define CH3_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x1B)
    #define CH3_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL3_MSB field mask & values */
    #define CH3_GCAL_MSB_GCAL3_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x1C (CH3_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL3_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH3_GCAL_LSB register address & default value */
    #define CH3_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x1C)
    #define CH3_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL3_LSB field mask & values */
    #define CH3_GCAL_LSB_GCAL3_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH3_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 4)

/* Register 0x1D (CH4_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE4[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX4[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH4_CFG register address & default value */
    #define CH4_CFG_ADDRESS                                                 ((uint8_t)  0x1D)
    #define CH4_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE4 field mask & values */
    #define CH4_CFG_PHASE4_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH4_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX4 field mask & values */
    #define CH4_CFG_MUX4_MASK                                               ((uint16_t) 0x0003)
    #define CH4_CFG_MUX4_AIN4P_AIN4N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH4_CFG_MUX4_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH4_CFG_MUX4_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH4_CFG_MUX4_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x1E (CH4_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL4_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH4_OCAL_MSB register address & default value */
    #define CH4_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x1E)
    #define CH4_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL4_MSB field mask & values */
    #define CH4_OCAL_MSB_OCAL4_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x1F (CH4_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL4_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH4_OCAL_LSB register address & default value */
    #define CH4_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x1F)
    #define CH4_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL4_LSB field mask & values */
    #define CH4_OCAL_LSB_OCAL4_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH4_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x20 (CH4_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL4_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH4_GCAL_MSB register address & default value */
    #define CH4_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x20)
    #define CH4_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL4_MSB field mask & values */
    #define CH4_GCAL_MSB_GCAL4_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x21 (CH4_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL4_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH4_GCAL_LSB register address & default value */
    #define CH4_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x21)
    #define CH4_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL4_LSB field mask & values */
    #define CH4_GCAL_LSB_GCAL4_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH4_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 5)

/* Register 0x22 (CH5_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE5[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX5[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH5_CFG register address & default value */
    #define CH5_CFG_ADDRESS                                                 ((uint8_t)  0x22)
    #define CH5_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE5 field mask & values */
    #define CH5_CFG_PHASE5_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH5_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX5 field mask & values */
    #define CH5_CFG_MUX5_MASK                                               ((uint16_t) 0x0003)
    #define CH5_CFG_MUX5_AIN5P_AIN5N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH5_CFG_MUX5_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH5_CFG_MUX5_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH5_CFG_MUX5_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x23 (CH5_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL5_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH5_OCAL_MSB register address & default value */
    #define CH5_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x23)
    #define CH5_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL5_MSB field mask & values */
    #define CH5_OCAL_MSB_OCAL5_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x24 (CH5_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL5_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH5_OCAL_LSB register address & default value */
    #define CH5_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x24)
    #define CH5_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL5_LSB field mask & values */
    #define CH5_OCAL_LSB_OCAL5_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH5_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x25 (CH5_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL5_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH5_GCAL_MSB register address & default value */
    #define CH5_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x25)
    #define CH5_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL5_MSB field mask & values */
    #define CH5_GCAL_MSB_GCAL5_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x26 (CH5_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL5_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH5_GCAL_LSB register address & default value */
    #define CH5_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x26)
    #define CH5_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL5_LSB field mask & values */
    #define CH5_GCAL_LSB_GCAL5_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH5_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 6)

/* Register 0x27 (CH6_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE6[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX6[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH6_CFG register address & default value */
    #define CH6_CFG_ADDRESS                                                 ((uint8_t)  0x27)
    #define CH6_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE6 field mask & values */
    #define CH6_CFG_PHASE6_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH6_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX6 field mask & values */
    #define CH6_CFG_MUX6_MASK                                               ((uint16_t) 0x0003)
    #define CH6_CFG_MUX6_AIN6P_AIN6N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH6_CFG_MUX6_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH6_CFG_MUX6_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH6_CFG_MUX6_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x28 (CH6_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL6_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH6_OCAL_MSB register address & default value */
    #define CH6_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x28)
    #define CH6_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL6_MSB field mask & values */
    #define CH6_OCAL_MSB_OCAL6_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x29 (CH6_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL6_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH6_OCAL_LSB register address & default value */
    #define CH6_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x29)
    #define CH6_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL6_LSB field mask & values */
    #define CH6_OCAL_LSB_OCAL6_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH6_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x2A (CH6_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL6_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH6_GCAL_MSB register address & default value */
    #define CH6_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x2A)
    #define CH6_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL6_MSB field mask & values */
    #define CH6_GCAL_MSB_GCAL6_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x2B (CH6_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL6_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH6_GCAL_LSB register address & default value */
    #define CH6_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x2B)
    #define CH6_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL6_LSB field mask & values */
    #define CH6_GCAL_LSB_GCAL6_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH6_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



#endif
#if (CHANNEL_COUNT > 7)

/* Register 0x2C (CH7_CFG) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                           PHASE7[9:0]                                                           |                   RESERVED0[3:0]                  |        MUX7[1:0]        |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH7_CFG register address & default value */
    #define CH7_CFG_ADDRESS                                                 ((uint8_t)  0x2C)
    #define CH7_CFG_DEFAULT                                                 ((uint16_t) 0x0000)

    /* PHASE7 field mask & values */
    #define CH7_CFG_PHASE7_MASK                                             ((uint16_t) 0xFFC0)

    /* RESERVED0 field mask & values */
    #define CH7_CFG_RESERVED0_MASK                                          ((uint16_t) 0x003C)

    /* MUX7 field mask & values */
    #define CH7_CFG_MUX7_MASK                                               ((uint16_t) 0x0003)
    #define CH7_CFG_MUX7_AIN7P_AIN7N                                        ((uint16_t) 0x0000 << 0)    // DEFAULT
    #define CH7_CFG_MUX7_ADC_INPUT_SHORT                                    ((uint16_t) 0x0001 << 0)
    #define CH7_CFG_MUX7_DC_DIAGNOSTIC                                      ((uint16_t) 0x0002 << 0)
    #define CH7_CFG_MUX7_AC_DIAGNOSTIC                                      ((uint16_t) 0x0003 << 0)



/* Register 0x2D (CH7_OCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                OCAL7_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH7_OCAL_MSB register address & default value */
    #define CH7_OCAL_MSB_ADDRESS                                            ((uint8_t)  0x2D)
    #define CH7_OCAL_MSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL7_MSB field mask & values */
    #define CH7_OCAL_MSB_OCAL7_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x2E (CH7_OCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             OCAL7_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH7_OCAL_LSB register address & default value */
    #define CH7_OCAL_LSB_ADDRESS                                            ((uint8_t)  0x2E)
    #define CH7_OCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* OCAL7_LSB field mask & values */
    #define CH7_OCAL_LSB_OCAL7_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH7_OCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)



/* Register 0x2F (CH7_GCAL_MSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                GCAL7_MSB[15:0]                                                                                                |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH7_GCAL_MSB register address & default value */
    #define CH7_GCAL_MSB_ADDRESS                                            ((uint8_t)  0x2F)
    #define CH7_GCAL_MSB_DEFAULT                                            ((uint16_t) 0x8000)

    /* GCAL7_MSB field mask & values */
    #define CH7_GCAL_MSB_GCAL7_MSB_MASK                                     ((uint16_t) 0xFFFF)



/* Register 0x30 (CH7_GCAL_LSB) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                             GCAL7_LSB[7:0]                                            |                                             RESERVED0[7:0]                                            |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CH7_GCAL_LSB register address & default value */
    #define CH7_GCAL_LSB_ADDRESS                                            ((uint8_t)  0x30)
    #define CH7_GCAL_LSB_DEFAULT                                            ((uint16_t) 0x0000)

    /* GCAL7_LSB field mask & values */
    #define CH7_GCAL_LSB_GCAL7_LSB_MASK                                     ((uint16_t) 0xFF00)

    /* RESERVED0 field mask & values */
    #define CH7_GCAL_LSB_RESERVED0_MASK                                     ((uint16_t) 0x00FF)


#endif

/* Register 0x3E (REGMAP_CRC) definition
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |   Bit 15   |   Bit 14   |   Bit 13   |   Bit 12   |   Bit 11   |   Bit 10   |    Bit 9   |    Bit 8   |    Bit 7   |    Bit 6   |    Bit 5   |    Bit 4   |    Bit 3   |    Bit 2   |    Bit 1   |    Bit 0   |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                                 REG_CRC[15:0]                                                                                                 |
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* REGMAP_CRC register address & default value */
    #define REGMAP_CRC_ADDRESS                                              ((uint8_t)  0x3E)
    #define REGMAP_CRC_DEFAULT                                              ((uint16_t) 0x0000)

    /* REG_CRC field mask & values */
    #define REGMAP_CRC_REG_CRC_MASK                                         ((uint16_t) 0xFFFF)



//****************************************************************************
//
// Channel data structure
//
//****************************************************************************

typedef struct
{
    uint16_t response;
    int16_t channel0;
    int16_t channel1;
    uint16_t crc;
} adc_channel_data;



//****************************************************************************
//
// Function prototypes
//
//****************************************************************************


//****************************************************************************
//
// Register macros
//
//****************************************************************************

/** Returns Number of Channels */
#define CHANCNT             ((uint8_t) ((g_ADSRegMap[ID_ADDRESS] & ID_CHANCNT_MASK) >> 8))

/** Revision ID bits */
#define REVISION_ID         ((uint8_t) ((g_ADSRegMap[ID_ADDRESS] & ID_REVID_MASK) >> 0))

/** Returns true if SPI interface is locked */
#define SPI_LOCKED(reg)          ((uint16_t) (reg & STATUS_LOCK_LOCKED))

/** Returns SPI Communication Word Format*/
#define WLENGTH             ((uint16_t) (g_ADSRegMap[MODE_ADDRESS] & MODE_WLENGTH_MASK))

/** Returns true if Register Map CRC byte enable bit is set */
#define REGMAP_CRC_ENABLED  ((uint16_t) (g_ADSRegMap[MODE_ADDRESS] & MODE_REG_CRC_EN_ENABLED))

/** Returns true if SPI CRC byte enable bit is set */
#define SPI_CRC_ENABLED     ((uint16_t) (g_ADSRegMap[MODE_ADDRESS] & MODE_RX_CRC_EN_ENABLED))

/** Returns false for CCITT and true for ANSI CRC type */
#define SPI_CRC_TYPE        ((uint16_t) (g_ADSRegMap[MODE_ADDRESS] & MODE_CRC_TYPE_MASK))

/** Data rate register field setting */
#define OSR_INDEX           ((uint8_t) ((g_ADSRegMap[CLOCK_ADDRESS] & CLOCK_OSR_MASK) >> 2))

/** Data rate register field setting */
#define POWER_MODE          ((uint8_t) ((g_ADSRegMap[CLOCK_ADDRESS] & CLOCK_PWR_MASK) >> 0))


#define SPI_FRAME_MODE_16BIT		(SPI_CTL0(SPI1)&SPI_CTL0_FF16)

#define ADS131M_MODE_CUR			(0xFFFF)		//使用当前模式寄存器值

//#define ADCSPI_FRAME_24BITS			(1u)
//#define ADCSPI_FRAME_16BITS			(0u)

#define ADC_DRDY_BANK				(GPIOC)
#define ADC_DRDY_PIN				(GPIO_PIN_6)

#define ADCSPI_CHECK_YES			(1u)
#define ADCSPI_CHECK_NONE			(0u)

/** ---	Start of Funtion define	---------------------------------------------------------------------- **/
void ADS131M_Test(void);
void ADS131M_Init(void);

ErrStatus ADS131M_Reset(void);
ErrStatus ADS131M_SoftReset(void);
ErrStatus ADS131M_WriteReg(uint8_t addr, uint16_t data, uint8_t bCheck);
//ErrStatus ADS131M_WriteMode(uint16_t mode);
ErrStatus ADS131M_SendCmd(uint16_t opcode, uint16_t *pRxData);
ErrStatus ADS131M_ReadReg(uint8_t addr, uint16_t *pRxData);
ErrStatus ADS131M_ReadData(adc_channel_data *pDataSt);
ErrStatus ADS131M_LockReg(void);
ErrStatus ADS131M_UnlockReg(void);

uint8_t ADS131M_GetFrameBytes(uint16_t mode);

void ADS131M_StartSample(void);
void ADS131M_StopSample(void);
void ADS131M_StartDMA(void);
void ADS131M_StopDMA(void);
ErrStatus ADS131M_QuerySmpNum(uint16_t *pBuf, uint8_t size);

void ADS131M_DMAMonitorDebris(void);
ErrStatus ADS131M_DMAGetDebrisOnce(tParticelInf *pstPtInf);

#endif //__ADS131M_H__
/** ---	End of File	---------------------------------------------------------------------------------- **/ 

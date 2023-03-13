#ifndef __HMC5883l_H
#define __HMC5883l_H
 
/* Includes ------------------------------------------------------------------*/
#include "iic.h"  

/* Exported macro ------------------------------------------------------------*/

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


/* Exported functions ------------------------------------------------------- */
void HMC5883lInit(void);
void HMC5883lRead(int16_t *mag_x, int16_t *mag_y, int16_t *mag_z);

#endif


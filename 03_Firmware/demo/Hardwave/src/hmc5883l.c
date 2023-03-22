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
#include "math.h"
/** ---	Start of Macro define 	---------------------------------------------------------------------  **/

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	芯片操作函数 -----------------------------------  **/

static int i2cWrite(uint8_t reg_, uint8_t Data)
{
	return HAL_I2C_Mem_Write( &hi2c2, HMC5883_WRITE_ADDR, reg_, I2C_MEMADD_SIZE_8BIT, &Data, 1, 1000);
	
}


static int i2cRead(uint8_t reg_, uint8_t len, uint8_t* buf)
{
	return HAL_I2C_Mem_Read( &hi2c2, HMC5883_READ_ADDR, reg_, I2C_MEMADD_SIZE_8BIT, buf, len, 1000);
}

/**
* @brief	hmc5883l初始化,包含传感器校准+++++++++++++++
* @note
*/
void HMC5883l_Init()
{
	i2cWrite(HMC5883_REG_CONFIGA, HMC5883_REG_CRA_MA_AVG_8 | HMC5883_REG_CRA_DO_Hz_75 | HMC5883_REG_CRA_MS_NORAL);
	i2cWrite(HMC5883_REG_CONFIGB, HMC5883_REG_GN_G820);
	i2cWrite(HMC5883_REG_MODE, HMC5883_REG_MODE_MR_CON);	
}

/**
* @brief	读取磁场传感器数据。
* @note
* @param	magData：存储磁场传感器原始数据的指针
*/
void HMC5883l_Read(int16_t *mag_x, int16_t *mag_y, int16_t *mag_z)
{
	uint8_t buf[6];
	i2cRead(HMC5883_REG_X_MSB, 6, buf);
	*mag_x = buf[0] << 8 | buf[1];
	*mag_z = buf[2] << 8 | buf[3];
	*mag_y = buf[4] << 8 | buf[5];
}

void HMC5883l_Angle_xz(double *angle_xz)
{
	int16_t m_x, m_y, m_z;
	HMC5883l_Read(&m_x, &m_y, &m_z);
	//atan2求出的angle取值范围为[-180,180]
    //angle_xy = atan2(y,x)*(180/3.14159265)+180;
   *angle_xz = atan2(m_x,m_z)*(180/3.14159265)+180;
    //angle_yz = atan2(y,z)*(180/3.14159265)+180;
}


/* 再通过角度范围判断，赋值相应的枚举值 */
unsigned char get_direction(void)
{
	unsigned char Direction;
	double angle_xz_val;
	HMC5883l_Angle_xz(&angle_xz_val);
    if (angle_xz_val >= 22 && angle_xz_val <= 67) {
        Direction = DIRECTION_NE;
    } else if (angle_xz_val >= 68  && angle_xz_val <= 111) {
        Direction = DIRECTION_E;
    } else if (angle_xz_val >= 112 && angle_xz_val <= 157) {
        Direction = DIRECTION_SE;
    } else if (angle_xz_val >= 158 && angle_xz_val <= 201) {
        Direction = DIRECTION_S;
    } else if (angle_xz_val >= 202 && angle_xz_val <= 247) {
        Direction = DIRECTION_SW;
    } else if (angle_xz_val >= 248 && angle_xz_val <= 291) {
        Direction = DIRECTION_W;
    } else if (angle_xz_val >= 292 && angle_xz_val <= 337) {
        Direction = DIRECTION_NW;
    } else {
        Direction = DIRECTION_N;
    }
    return Direction;
 
}
 
/* 通过枚举值赋值相应的字符串，实现通过角度判断方位 */
char* get_direction_str()
{
    switch(get_direction()) {
        case DIRECTION_NE: return "NE";
        case DIRECTION_E:  return "E";
        case DIRECTION_SE: return "SE";
        case DIRECTION_S:  return "S.";
        case DIRECTION_SW: return "SW";
        case DIRECTION_W:  return "W";
        case DIRECTION_NW: return "NW";
        case DIRECTION_N:  return "N";
    }
    return 0;
}


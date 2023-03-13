#ifndef _TIMER_H_
#define _TIMER_H_

#include "gd32f4xx.h"

//#define PWM_FREQ_266KHZ
#define PWM_FREQ_466KHZ
//#define PWM_FREQ_700KHZ
//#define PWM_FREQ_933KHZ

#ifdef PWM_FREQ_266KHZ				//实际为265.8kHz
#define PWM_FREQ_DIV_NUM	(316)	//必须为偶数，Timer的分频倍数：168000/466/2 ≈ 316
#define PWM_ORG_PHASE_PULSE	(256)	//必须大于DIV_NUM的一半，以便90°超前相位
#endif

#ifdef PWM_FREQ_466KHZ				//实际为466.667kHz
#define PWM_FREQ_DIV_NUM	(180)	//必须为偶数，Timer的分频倍数：168000/266/2 ≈ 180
#define PWM_ORG_PHASE_PULSE	(128)	//必须大于DIV_NUM的一半，以便90°超前相位
#endif

#ifdef PWM_FREQ_700KHZ				//实际为700kHz
#define PWM_FREQ_DIV_NUM	(120)	//必须为偶数，Timer的分频倍数：168000/700/2 = 120
#define PWM_ORG_PHASE_PULSE	(90)	//必须大于DIV_NUM的一半，以便90°超前相位
#endif

#ifdef PWM_FREQ_933KHZ				//实际为933.333kHz
#define PWM_FREQ_DIV_NUM	(90)	//必须为偶数，Timer的分频倍数：168000/932/2 ≈ 90
#define PWM_ORG_PHASE_PULSE	(64)	//必须大于DIV_NUM的一半，以便90°超前相位
#endif

#define ANGLE_PERIOD		(1024)	 //每周角度计数，按照1024等分，每等分约为0.352°；这样可便于查表45°~90°共128点，也便于使用整数乘法计算角度
#define PWM_PERIOD			((uint16_t) (PWM_FREQ_DIV_NUM*2))
#define PWM_PSC				(1)		//不预分频
#define PI					((float)3.1415926535897932384626433832795)
#define PI2					(2*PI)

#define COEF_TANSCALE2RADIAN	((float)(PI2/ANGLE_PERIOD))	//将Tan 1024分度转换为弧度
#define COEF_RADIAN2ANGLE		((float)(180.0f/PI))				//将弧度转换为角度
#define COEF_RADIAN2PWMSCALE	((float)(PWM_PERIOD/PI2))	//将弧度转换为PWM分度
#define COEF_PWMSCALE2ANGLE		((float)(360.0f/PWM_PERIOD)) 	//将PWM 632分度转换为角度
#define COEF_PWMSCAL2RADIAN		((float)(PI2/PWM_PERIOD))	//将PWM分度转换为弧度
#define COEF_ANGLE2RADIAN		((float)(PI/180.0f))				//将角度转换为弧度
#define COEF_TANSCALE2ANGLE		(COEF_TANSCALE2RADIAN*COEF_RADIAN2ANGLE)	//将Tan 1024分度转换为角度

#define ANGLE_SYNC_LOW	(PWM_FREQ_DIV_NUM+PWM_ORG_PHASE_PULSE)		//同相时的角度最小值（分度值）
#define ANGLE_SYNC_HIGH	(PWM_ORG_PHASE_PULSE)							//同相时的角度最大值（分度值）

void Timer0_Init(void);		//用于正弦生成的PWM波形产生
void Timer0_SetCpsPhase(uint16_t phase);
uint16_t Timer0_GetCpsPhase(void);

void Timer2_Init(void);		//用于接收SPI \DRDY，并产生\CS
void Timer3_Init(void);		//用于根据\CS，产生SPI SCLK
void EXADC_Timer_Start(void);	//将SPI接口改为Timer控制模式
void EXADC_Timer_Stop(void);	//将SPI接口改为SPI控制模式
void EXADC_TimerDrdyEnable(void);	//使能DRDY触发，开始采集

void Timer4_Init(void);			//用于ADC采样周期设置
void Timer4_SetPeriod(unsigned short period);	//设置内部ADC采样周期

void Timer7_Init(void);		//用于为ADS131M提供时钟信号


#endif



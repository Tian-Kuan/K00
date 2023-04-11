#ifndef _TIMER_H_
#define _TIMER_H_

#include "gd32f4xx.h"

//#define PWM_FREQ_266KHZ
#define PWM_FREQ_466KHZ
//#define PWM_FREQ_700KHZ
//#define PWM_FREQ_933KHZ

#ifdef PWM_FREQ_266KHZ				//ʵ��Ϊ265.8kHz
#define PWM_FREQ_DIV_NUM	(316)	//����Ϊż����Timer�ķ�Ƶ������168000/466/2 �� 316
#define PWM_ORG_PHASE_PULSE	(256)	//�������DIV_NUM��һ�룬�Ա�90�㳬ǰ��λ
#endif

#ifdef PWM_FREQ_466KHZ				//ʵ��Ϊ466.667kHz
#define PWM_FREQ_DIV_NUM	(180)	//����Ϊż����Timer�ķ�Ƶ������168000/266/2 �� 180
#define PWM_ORG_PHASE_PULSE	(128)	//�������DIV_NUM��һ�룬�Ա�90�㳬ǰ��λ
#endif

#ifdef PWM_FREQ_700KHZ				//ʵ��Ϊ700kHz
#define PWM_FREQ_DIV_NUM	(120)	//����Ϊż����Timer�ķ�Ƶ������168000/700/2 = 120
#define PWM_ORG_PHASE_PULSE	(90)	//�������DIV_NUM��һ�룬�Ա�90�㳬ǰ��λ
#endif

#ifdef PWM_FREQ_933KHZ				//ʵ��Ϊ933.333kHz
#define PWM_FREQ_DIV_NUM	(90)	//����Ϊż����Timer�ķ�Ƶ������168000/932/2 �� 90
#define PWM_ORG_PHASE_PULSE	(64)	//�������DIV_NUM��һ�룬�Ա�90�㳬ǰ��λ
#endif

#define ANGLE_PERIOD		(1024)	 //ÿ�ܽǶȼ���������1024�ȷ֣�ÿ�ȷ�ԼΪ0.352�㣻�����ɱ��ڲ��45��~90�㹲128�㣬Ҳ����ʹ�������˷�����Ƕ�
#define PWM_PERIOD			((uint16_t) (PWM_FREQ_DIV_NUM*2))
#define PWM_PSC				(1)		//��Ԥ��Ƶ
#define PI					((float)3.1415926535897932384626433832795)
#define PI2					(2*PI)

#define COEF_TANSCALE2RADIAN	((float)(PI2/ANGLE_PERIOD))	//��Tan 1024�ֶ�ת��Ϊ����
#define COEF_RADIAN2ANGLE		((float)(180.0f/PI))				//������ת��Ϊ�Ƕ�
#define COEF_RADIAN2PWMSCALE	((float)(PWM_PERIOD/PI2))	//������ת��ΪPWM�ֶ�
#define COEF_PWMSCALE2ANGLE		((float)(360.0f/PWM_PERIOD)) 	//��PWM 632�ֶ�ת��Ϊ�Ƕ�
#define COEF_PWMSCAL2RADIAN		((float)(PI2/PWM_PERIOD))	//��PWM�ֶ�ת��Ϊ����
#define COEF_ANGLE2RADIAN		((float)(PI/180.0f))				//���Ƕ�ת��Ϊ����
#define COEF_TANSCALE2ANGLE		(COEF_TANSCALE2RADIAN*COEF_RADIAN2ANGLE)	//��Tan 1024�ֶ�ת��Ϊ�Ƕ�

#define ANGLE_SYNC_LOW	(PWM_FREQ_DIV_NUM+PWM_ORG_PHASE_PULSE)		//ͬ��ʱ�ĽǶ���Сֵ���ֶ�ֵ��
#define ANGLE_SYNC_HIGH	(PWM_ORG_PHASE_PULSE)							//ͬ��ʱ�ĽǶ����ֵ���ֶ�ֵ��

void Timer0_Init(void);		//�����������ɵ�PWM���β���
void Timer0_SetCpsPhase(uint16_t phase);
uint16_t Timer0_GetCpsPhase(void);

void Timer2_Init(void);		//���ڽ���SPI \DRDY��������\CS
void Timer3_Init(void);		//���ڸ���\CS������SPI SCLK
void EXADC_Timer_Start(void);	//��SPI�ӿڸ�ΪTimer����ģʽ
void EXADC_Timer_Stop(void);	//��SPI�ӿڸ�ΪSPI����ģʽ
void EXADC_TimerDrdyEnable(void);	//ʹ��DRDY��������ʼ�ɼ�

void Timer4_Init(void);			//����ADC������������
void Timer4_SetPeriod(unsigned short period);	//�����ڲ�ADC��������

void Timer7_Init(void);		//����ΪADS131M�ṩʱ���ź�


#endif



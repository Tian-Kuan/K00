/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               pwm.c
** Latest modified Date:    2022-6-7
** Latest Version:          1.0
** Descriptions:            To Control the Driver Signals
**							
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-6-7
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

#include "gd32f4xx.h"
#include "systick.h"

#include "pwm.h"
#include "timer.h"
#include "console.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/


/** ---	Start of Para define 	---------------------------------------------------------------------  **/
#define ATAN_TABLE_LEN	(ANGLE_PERIOD>>3)

//atg�Ƕȱ�����Բ�ܷ�Ϊ1024�ݣ�����Ϊ���е�128�ݣ�����0~45�������
//��������Ϊʵ�����ݷŴ�128��, ��ֵʵ��Ϊ: ctg((��Ӧ���+0.5)/1024*2*PI)*128
//����������С�ڵ��ڶ�Ӧ��ֵʱ����ʾ�Ƕ�Ϊ(��Ӧ���/1024*2*PI)
const uint16_t g_ATANTable[] = {0xA2F9,0x3652,0x2097,0x1747,0x121A,0x0ECF,0x0C87,0x0ADB,
								0x0993,0x0891,0x07BF,0x0712,0x0681,0x0605,0x059A,0x053D,
								0x04EB,0x04A3,0x0462,0x0428,0x03F4,0x03C4,0x0399,0x0371,
								0x034D,0x032B,0x030C,0x02EF,0x02D4,0x02BB,0x02A3,0x028D,
								0x0279,0x0265,0x0253,0x0242,0x0231,0x0222,0x0213,0x0205,
								0x01F8,0x01EB,0x01DF,0x01D4,0x01C9,0x01BE,0x01B4,0x01AA,
								0x01A1,0x0198,0x018F,0x0187,0x017F,0x0177,0x0170,0x0169,
								0x0162,0x015B,0x0155,0x014E,0x0148,0x0142,0x013D,0x0137,
								0x0132,0x012D,0x0128,0x0123,0x011E,0x0119,0x0115,0x0110,
								0x010C,0x0108,0x0104,0x0100,0x00FC,0x00F8,0x00F4,0x00F1,
								0x00ED,0x00EA,0x00E6,0x00E3,0x00E0,0x00DD,0x00DA,0x00D7,
								0x00D4,0x00D1,0x00CE,0x00CB,0x00C8,0x00C6,0x00C3,0x00C0,
								0x00BE,0x00BB,0x00B9,0x00B6,0x00B4,0x00B2,0x00AF,0x00AD,
								0x00AB,0x00A9,0x00A7,0x00A5,0x00A2,0x00A0,0x009E,0x009C,
								0x009A,0x0099,0x0097,0x0095,0x0093,0x0091,0x008F,0x008E,
								0x008C,0x008A,0x0088,0x0087,0x0085,0x0083,0x0082,0x0080	
								};

void PWM_ScanTest(void);
void PWM_CpsRevise(void);
void PWM_EnableTest(void);
								
tdCS_TestList func_pwm_test[]=
{
	{PWM_EnableTest,	"Test Enable or Disable PWM CH" },
	{PWM_ScanTest,	  	"Test PWM Phase Scan (Timer0)"  },
	{PWM_ManuAdjust,	"Compensate Phase Manual Adjust"},
	{PWM_CpsRevise,		"Test Switching Pulse Setting"  },	
	{0,0}
};
/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

								
/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	PWM_Init
*	�������ܣ�	��PWMʹ�õ���Timer���г�ʼ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_Init(void)
{
	Timer0_Init();
}

/************************************************************
*	�������ƣ�	PWM_SetCpsPhase
*	�������ܣ�	PWM���������źźͳ�ʼ�����źŵ���λ��������λ���Բ��������ź�
*				�����ʵ����λӦ���ڡ�180�㣨����512���䣬������֮�������360�㣨����1024��
*	��ڲ�����	fDstPhase	- Ŀ����λ��һ��Ϊ�����ź���λ, ��λ ����
*				fCpsPhaseOrg- �����źų�ʼ��λ���ڲ�����180������ǰ����λ��,��λ ����
*	���ز�����	��ǰʵ�����õ�CPS��λ����λ������
*	˵����	    CPS��λ = DRV��λ-CPS��ʼ��λ������Ǹ���λ��Ӧ��һ�����ڣ�����ǰΪ��
*				���ڲ�õĲ�����λ�Ѿ�������180�㣬�����Ҫ�Բ�����λ����Ϊ����ǰ����λ������
************************************************************/
float PWM_SetCpsPhase(float fDstPhase, float fCpsPhaseOrg)
{	
	int16_t iPwmPhase;
	float fPwmPhase;
	
	fPwmPhase = (fDstPhase - fCpsPhaseOrg)*COEF_RADIAN2PWMSCALE;
	iPwmPhase = (int16_t) (fPwmPhase + ((fPwmPhase<0)?-0.5:0.5));	//��������
	
	while(iPwmPhase<0)
		iPwmPhase += PWM_PERIOD;
	while(iPwmPhase>=PWM_PERIOD)
		iPwmPhase -= PWM_PERIOD;

	Timer0_SetCpsPhase((uint16_t) iPwmPhase);
	
	if( iPwmPhase >= (PWM_PERIOD/2))
		iPwmPhase -= PWM_PERIOD;	//��Ϊ������λ
	return (iPwmPhase*COEF_PWMSCAL2RADIAN);
}

/************************************************************
*	�������ƣ�	PWM_TanScale
*	�������ܣ�	����I/Q��ֵ��ȷ���ź���λ
*	��ڲ�����	iSin	- I����
*				iCos	- Q����
*	���ز�����	����Tan����ֶ�: ��Χ -512~511
*	˵����	    �Ƕȵļ��ԣ���ǰΪ�����ӳ�Ϊ��
************************************************************/
int16_t PWM_TanScale(int16_t iSin, int16_t iCos)
{
	uint16_t sinAbs, cosAbs;
	uint8_t  quad=0;	// 0x11 - һ���ޣ�0x10 - �����ޣ�0x00 - �����ޣ�0x01 - ������
	uint8_t  isTan=0;		//����1������0
	uint16_t iMax, iMin;
	uint32_t u32Tan;
	uint16_t u16Tan;
	int16_t iAngle;
	
	//����ȷ��
	if(iSin>=0)	//һ��������
	{
		sinAbs =(uint16_t) iSin;	//ֻʹ�ø�16bit
		quad |= 0x10;
	}
	else
		sinAbs =(uint16_t) (-iSin);	
	if(iCos>=0)	//һ��������
	{
		cosAbs = (uint16_t) iCos;
		quad |= 0x01;
	}
	else
		cosAbs = (uint16_t) (-iCos);
	
	//ȷ�����л�������
	if(sinAbs>=cosAbs)	//����
	{
		iMax = sinAbs;
		iMin = cosAbs;
		isTan = 1;
	}
	else
	{
		iMax = cosAbs;
		iMin = sinAbs;
		isTan = 0;
	}
	
	if(iMin != 0)	//��Сֵ��Ϊ0������������
	{
		u32Tan = (uint32_t) (iMax<<7);
		u32Tan /= iMin;	
	}
	else
		u32Tan = 0xFFFF;

	//ת��Ϊ16bit���бȽ�
	if(u32Tan > g_ATANTable[0])
	{
		if(isTan)	//����
			iAngle = ANGLE_PERIOD>>2;	//90��
		else		//����
			iAngle = 0;
	}
	else	//���ַ����
	{
		iAngle = 0;
		u16Tan = (uint16_t) u32Tan;
		iMin = 7;
		while(iMin--)	//����iMin��iMax��Ϊ��ͨ����
		{
			iMax = iAngle | (0x1<<iMin);
			if( u16Tan <= g_ATANTable[iMax])	//�Ȳ��ֵС��������
				iAngle = iMax;							
		}
		
		iAngle++;	//�Ƕ�Ϊ���+1
		if(isTan)	//���У���90���
			iAngle = (ANGLE_PERIOD>>2)-iAngle;
	}
	switch(quad)
	{
		case 0x10:	//�ڶ�����
			if(iAngle)	//�ǶȲ�Ϊ0
				iAngle = (ANGLE_PERIOD>>1)-iAngle;	    //180���
			else		//�Ƕ�Ϊ0ʱ����Ϊ-180��
				iAngle = -(ANGLE_PERIOD>>1);			//����Ƕȷ�Χ-512~511
			break;
		case 0x00:	//��������
			iAngle = iAngle - (ANGLE_PERIOD>>1);	//��180��
			break;
		case 0x01:	//��������
			iAngle = -iAngle;				//ȡ��
			break;			
	}
	return iAngle;
}

/************************************************************
*	�������ƣ�	PWM_Tan
*	�������ܣ�	����I/Q��ֵ��ȷ���ź���λ
*	��ڲ�����	iSin	- I����
*				iCos	- Q����
*	���ز�����	���ػ���ֵ: ��Χ ��PI
*	˵����	    �Ƕȵļ��ԣ���ǰΪ�����ӳ�Ϊ��
************************************************************/
float PWM_Tan(int16_t iSin, int16_t iCos)
{	
	return (PWM_TanScale(iSin, iCos)*COEF_TANSCALE2RADIAN);	
}

/************************************************************
*	�������ƣ�	PWM_GetCpsPhase
*	�������ܣ�	��ȡ��ǰ���ò����źŵ���λ
*	��ڲ�����	None
*	���ز�����	��ǰ�����źŵ���λ���ã���λ�����ȣ���Χ��0~2��
*	˵����	    
************************************************************/
float PWM_GetCpsPhase(void)
{
	return (Timer0_GetCpsPhase()*COEF_PWMSCAL2RADIAN);
}

/************************************************************
*	�������ƣ�	PWM_Enable
*	�������ܣ�	ʹ�ܻ��߽��� ����������ͨ��
*	��ڲ�����	state	- PWM_DRIVER_EN, ʹ������ͨ��
*						- PWM_COMPENSATE_EN, ʹ�ܲ���ͨ��
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_Enable(uint8_t state)
{
    if(state & PWM_DRIVER_EN)
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_ENABLE);
    else
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_DISABLE);
    
    if(state & PWM_COMPENSATE_EN)
        timer_channel_output_state_config(TIMER0, TIMER_CH_3, TIMER_CCX_ENABLE);
    else
        timer_channel_output_state_config(TIMER0, TIMER_CH_3, TIMER_CCX_DISABLE);        
}

/************************************************************
*	�������ƣ�	PWM_GetEnableFlag
*	�������ܣ�	��ȡPWMͨ��ʹ��״̬
*	��ڲ�����	None
*	���ز�����	state	- PWM_DRIVER_EN, ʹ������ͨ��
*						- PWM_COMPENSATE_EN, ʹ�ܲ���ͨ��
*	˵����	    
************************************************************/
uint8_t PWM_GetEnableFlag(void)
{
	uint8_t state=0;
	if(	TIMER_CHCTL2(TIMER0) & TIMER_CHCTL2_CH3EN)
		state |= PWM_COMPENSATE_EN;
	if(	TIMER_CHCTL2(TIMER0) & TIMER_CHCTL2_CH0EN)
		state |= PWM_DRIVER_EN;
	return state;
}

/************************************************************
*	�������ƣ�	PWM_EnableTest
*	�������ܣ�	��PWM���ͨ�����н��ܻ���ʹ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_EnableTest(void)
{
	char ch;
	uint8_t state;
	
	state = PWM_GetEnableFlag();
	cprintf("Enable the Driver Channel of PWM?(Y/N)(Default: %c)\n",(state&PWM_DRIVER_EN)?'Y':'N');
	ch=CS_Getch();
	cprintf("  PWM Driver: ");
	if(state&PWM_DRIVER_EN)	//Ĭ��Ϊʹ��
	{
		if( ch=='N' || ch=='n')
			state &= ~PWM_DRIVER_EN;
	}
	else if(ch=='Y' || ch=='y')
		state |= PWM_DRIVER_EN;
		
	if(state&PWM_DRIVER_EN)
		cprintf("ENABLED\n");
	else
		cprintf("DISABLED\n");
	
	cprintf("Enable the Compensation Channel of PWM?(Y/N)(Default: %c)\n",(state&PWM_COMPENSATE_EN)?'Y':'N');
	ch=CS_Getch();
	cprintf("  PWM Compensation: ");
	if(state&PWM_COMPENSATE_EN)	//Ĭ��Ϊʹ��
	{
		if( ch=='N' || ch=='n')
			state &= ~PWM_COMPENSATE_EN;
	}
	else if(ch=='Y' || ch=='y')
		state |= PWM_COMPENSATE_EN;
		
	if(state&PWM_COMPENSATE_EN)
		cprintf("ENABLED\n");
	else
		cprintf("DISABLED\n");
	
	PWM_Enable(state);
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	PWM_ScanTest
*	�������ܣ�	��PWM��������źŵ���λ����ɨ�����
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_ScanTest(void)
{
    int iData;
    uint32_t iDelay = 100;
	int16_t iStep = 1;
	float fStep, fPhase = 0;
	char ch;
    cprintf("\nTest Scan the PWM Compensation Signal phase\n");
    cprintf("Enter the Interval Time for each Scan(range: 1~1000ms)(Default: %dms): ", iDelay);
    if(ERROR != CS_GetIntNum(&iData))
    {
        if( iData>=1 && iData<=1000)
            iDelay = (uint32_t) iData;
    }    
    cprintf("  Set Interval Time: %dms\n", iDelay);
    
    cprintf("Enter the Phase change per step(range: -180~180��)(Default: %d): ", iStep);
    if(ERROR != CS_GetIntNum(&iData))
    {
        if( iData>=-180 && iData<=180)
            iStep = (int16_t) iData;
    }
    cprintf("  Set Steps: %d\n", iStep);
	PWM_SetCpsPhase( fPhase, 0);	//���ó�ʼ��λ
	//��Ϊ����
	fStep = iStep*COEF_ANGLE2RADIAN;
    
    cprintf("Scan Now, Press Any Key to Stop...\n");
    
	while(ERROR == CS_QueryChar(&ch))
	{
		fPhase += fStep;	//�仯һ����λ
		if(fPhase > PI)
			fPhase -= 2*PI;	//����180�㣬�����360��
		else if(fPhase < -PI)
			fPhase += 2*PI;	//����-180�㣬�����360��
		delay_1ms(iDelay);
		PWM_SetCpsPhase( fPhase, 0);		
    }
	cprintf("The Last Phase Set As: %.1f ��, Please Check\n", fPhase*COEF_RADIAN2ANGLE);
    cprintf("End of Scan Phase\n");
}

/************************************************************
*	�������ƣ�	PWM_ManuAdjust
*	�������ܣ�	�ֶ�������λ
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_ManuAdjust(void)
{
	uint16_t iStep = 1;
	int16_t  iCnt;
	int iInput;
	char ch = 0;
	float fPhase;
	
	cprintf("\nAdjust Compensation Phase Manually\n");
	cprintf("Enter the Step for Adjust(Each 360/%d degree)(Range: 1~79)(Default: %d): ", PWM_PERIOD, iStep);
	if(ERROR != CS_GetIntNum(&iInput))
	{
		if(iInput>=1 && iInput<=79)
			iStep = iInput;
	}
	fPhase = iStep*COEF_PWMSCALE2ANGLE;
	cprintf("  Set Step As: %d (%.1f ��)\n", iStep, fPhase);
	
	iCnt = (int16_t) (PWM_GetCpsPhase()*COEF_RADIAN2PWMSCALE+0.5f);
	
	fPhase = iCnt*COEF_PWMSCALE2ANGLE;
	cprintf("Original Phase is: %.1f(%d/%d)\n", fPhase, iCnt, PWM_PERIOD);
	cprintf("Press [+] for Up Step And [-] for Down Step, Others Exit\n");
	while(1)
	{
		if(ERROR == CS_QueryChar(&ch))
			continue;
		if(ch=='+')
			iCnt += iStep;
		else if(ch=='-')
			iCnt -= iStep;
		else
			break;	
		PWM_SetCpsPhase(iCnt*COEF_PWMSCAL2RADIAN,0);
		fPhase = iCnt*COEF_PWMSCALE2ANGLE;			
		cprintf("  %.1f �� (%d/%d)\n", fPhase, iCnt, PWM_PERIOD);
	}	
	cprintf("End of Manual Copensation Adjust\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	PWM_CpsRevise
*	�������ܣ�	ֱ�Ӹ����������λ�ı䲹���ź����������PWM�ֶ�����
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_CpsRevise(void)
{
	int iInput; 
	int16_t pulse;
	
	cprintf("\nRevise the Compensation Pulse Directly with PWM Scale and Test Switching Pulse between phases\n");
	cprintf("Note: The Phase input not according to driver pwm\n");
	
	//������ת��Ϊ����
	pulse = PWM_ORG_PHASE_PULSE - (int16_t)(PWM_GetCpsPhase()*COEF_RADIAN2PWMSCALE+0.5f);
	if(pulse<0)
		pulse += PWM_PERIOD;
	cprintf("Current Pulse: %d\n", pulse);
	
	while(1)
	{
		cprintf("Input New Pulse In PWM Scale( Range: 0(0��)~%d(approximate 360��)): ", PWM_PERIOD-1);
		if(ERROR == CS_GetIntNum(&iInput) || iInput<0 || iInput>=PWM_PERIOD)
		{
			cprintf("  Invalid Input, Exit\n");
			break;
		}
		
		//������ת��Ϊ����
		pulse = PWM_ORG_PHASE_PULSE - (int16_t)(PWM_GetCpsPhase()*COEF_RADIAN2PWMSCALE+0.5f);
		if(pulse<0)
			pulse += PWM_PERIOD;
		cprintf("  Current Pulse: %d\n", pulse);

		pulse = (int16_t) iInput;		
		cprintf("  Set the New Pulse As: %d\n", pulse);
		
		//����λת��ΪPWM�����򣬱�֤�������λ�������õ����壨��Timer0_SetCpsPhase�У������Ϊʵ����λ���������壩
		if(pulse<=ANGLE_SYNC_HIGH)	//����ͬ����
			pulse = PWM_ORG_PHASE_PULSE - pulse; 			//��λԽ����ͬ����ԽԶ�����õ�Timer��
		else
			pulse = PWM_PERIOD+PWM_ORG_PHASE_PULSE-pulse;
		
		Timer0_SetCpsPhase((uint16_t)pulse);
	}	
	
	cprintf("End of Compensation Phase Revising\n");
	CS_WaitKey();
}

/************************************************************
*	�������ƣ�	PWM_Test
*	�������ܣ�	����PWM���ܲ��ԣ�չʾ���Ŀ¼
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void PWM_Test(void)
{
    cprintf("\nTest PWM Functions\n");
    MENU_Disp(func_pwm_test);  
    cprintf("End of PWM Function Test\n");
}

//void PWM_CmpTan(void)
//{
//	int32_t iSin, iCos;
//	double fAngle;
//	int16_t iAngle;
//	
//	//���������㷨: ����100 020 001��
//	for(iSin = -5000; iSin<=5000; iSin+=10)
//	{
//		for(iCos=-5000; iCos<=5000; iCos+=10)
//			fAngle = atan2((double)iCos, (double)iSin);
//	}
//	//������: ����100 020 001��
//	for(iSin = -5000; iSin<=5000; iSin+=10)
//	{
//		for(iCos=-5000; iCos<=5000; iCos+=10)
//			fAngle = PWM_Tan(iSin, iCos);
//	}	
//}



/** ---	End of File ---------------------------------------------------------------------------------  **/ 

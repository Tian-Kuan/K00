/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               pwm.c
** Latest modified Date:    2022-5-25
** Latest Version:          1.0
** Descriptions:            Used for PWM Control Test (Timer0) & Timer2 For InnerADC & Timer7 For ADS131M
**                          Timer0 Interface Define
**                          PE09 - PWM_D    CH0
**                          PE10 - DEM_I-   CH1
**                          PE11 - DEM_I+   
**                          PE12 - DEM_Q-   CH2
**                          PE13 - DEM_Q+
**                          PE14 - PWM_C    CH3
**--------------------------------------------------------------------------------------------------------
** Created by:              TianKuan
** Created date:            2022-5-5
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Yong
** Modified date:           2022-10-26
** Version:                 1.1
** Descriptions:            1. �޸�Timer2��Timer3��;
**							����	�ĺ���;				��ǰ��;
**							Timer0	ͬ��ǰ					������Ȧ�����������źŵĲ��������������
**							Timer2  ����ADC DRDY, ����CS	����CPU�ڲ�ADC�ĵ�ת���ж�Ƶ�ʼ�ʱ
**							Timer3  ����CS, ����SCLK		���ڶ�ADCƬѡ�źŵļ�ʱ��λ
**							Timer4  ͬ��ǰ��Timer2			��
**							Timer7  ͬ��ǰ					�������ⲿADC�ṩ��ʱ��					
**
**							2. ԭΪ266kHzƵ�ʣ����Ӷ�466��699��932kHz��֧��
**
*********************************************************************************************************/

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"
#include "systick.h"

#include "timer.h"
#include "console.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
typedef struct
{
    uint16_t phase;     //��������λ/��ǰ��λ
    uint8_t  polarity;  //�����ü���/��ǰ���ԣ�0��������ͬ��1������
    uint8_t  exec;      //ִ�����ݣ�ÿ2bit����һ������2bit��ʼ��00 - ֱ��ִ�У� 10 - �跭ת
} tPWMCpsSet;


#define PWM_CPSSET_POL_ANTI    (1)    //����
#define PWM_CPSSET_POL_SAME    (0)    //ͬ��

#define PWM_CPSSET_EXEC_NONE     (0x0)   //�޴�ִ������
//����������Ҫ�ȴ�0~1���жϣ�����ȫ��������ֱ���޸����ݿɱ������޸Ĳ�������
#define PWM_CPSSET_EXEC_REVISE 	 (PWM_CPSSET_EXEC_NONE)			//ֱ���޸�ʱ�����õȴ����ɱ������޸Ĳ���ĵ�
#define PWM_CPSSET_EXEC_REVERSE  (0x2)							//��0��ת���ȴ�һ���ж�

/** ---	Start of Para define 	---------------------------------------------------------------------  **/
static tPWMCpsSet g_stPwmCpsSet;

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	Timer0_Init
*	�������ܣ�	��ʼ������ Timer0������ģ���źŲ��������������
*	��ڲ�����	
*	���ز�����	None
*	˵����	    PE09 - PWM_D    CH0
*               PE10 - DEM_I-   CH1��Sin���ͺ�90�㣩
*               PE11 - DEM_I+   
*               PE12 - DEM_Q-   CH2��Cos��ͬCH0��λ��
*               PE13 - DEM_Q+
*               PE14 - PWM_C    CH3
*				Timer0ʹ��PCLK2 = AHB/2, ����MUL2����Timer_Clk = AHB��168MHz 
************************************************************/
void Timer0_Init(void)
{
	timer_parameter_struct timer_initpara;	
	timer_oc_parameter_struct timer_ocintpara;	
	
    //--------------GPIO����--------------
	rcu_periph_clock_enable(RCU_GPIOE);     	            //ʹ��GPIOEʱ��	
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE,      //���ù�������PE9~14(TIMER0_CH0)  PE9~14���á�����
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
	gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,    	//����ioģʽ
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
	gpio_af_set(GPIOE, GPIO_AF_1,                           //�������Ÿ��ù���,��������ΪGPIO_AF_1����ʱ��0
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);

    //---------------��ʱ������---------------	
	rcu_periph_clock_enable(RCU_TIMER0);    //ʹ�ܶ�ʱ��ʱ��
		
	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//����PCLK2ΪAHB/2��MUL2��Timerʱ��ΪAHB: 168MHz
	
	timer_deinit(TIMER0);   //��λ��ʱ��	
	
	timer_primary_output_config(TIMER0, ENABLE);	//ʹ�ܻ������
		
	timer_initpara.prescaler         = PWM_PSC-1;           //��ʱ����Ƶ	
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//���ض���ģʽ
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;    //���ϼ���ģʽ
	timer_initpara.period            = PWM_FREQ_DIV_NUM-1;  //�Զ���װ��ֵ	
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;    //ʱ�ӷ�Ƶϵ��	
	timer_initpara.repetitioncounter = 0;                   //�����ظ�������ֵ	
	timer_init(TIMER0,&timer_initpara);
		
    //----------------��ʼ��PWMͨ��-----------------	
    //CH1&CH2 DEM_I/DEM_Q ���������WNOTE������ͨ������ͨ��֮���������Ч�ǽ���ģ�����ͨ�����У��򻥲���Ч
	//WNOTE: ����TMUX6111Ϊ0��1�أ����ע�����Ӧ��PWM_D��PWM_C�෴����ͬʱDEM_IӦ���ͺ�DEM_Q90�㣬���ܵõ���ȷ��Sin/Cos��λ
	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;        //�Ƚ����ʹ��	
	timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;       //�Ƿ񻥲�ͨ�����	
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;	//�����Ч����: ��
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_LOW;  //������Ч���ԣ���
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH; //����״̬��ͨ��������ԣ���
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;//����ʱ����ͨ��������ԣ���
	timer_channel_output_config(TIMER0,TIMER_CH_1,&timer_ocintpara);
	timer_channel_output_config(TIMER0,TIMER_CH_2,&timer_ocintpara); 

    //CH0&CH3 PWM_D/PWM_C ��·�Ƚ����
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;      //���ǻ���ͨ�����	
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;	//�������: ��	
	timer_channel_output_config(TIMER0,TIMER_CH_0,&timer_ocintpara);
	timer_channel_output_config(TIMER0,TIMER_CH_3,&timer_ocintpara);		

	//���������ת����ֵ��
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,PWM_ORG_PHASE_PULSE);	//PE9 -->PWM_Dri
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,PWM_ORG_PHASE_PULSE-(PWM_FREQ_DIV_NUM>>1));	//PE11-->Sin_Carr, �ͺ�Cos90�㣬���ڿ���Ϊ�����ƣ�ʵ�ʳ�ǰ90��
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,PWM_ORG_PHASE_PULSE);	//PE13-->Cos_Carr
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_3,PWM_ORG_PHASE_PULSE);	//PE14-->PWM_Comp
		
	//���ģʽ����Ϊ��תģʽ
	timer_channel_output_mode_config(TIMER0,TIMER_CH_0,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_1,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_3,TIMER_OC_MODE_TOGGLE);
		
	//��ʹ��Ӱ�ӼĴ���
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_3,TIMER_OC_SHADOW_ENABLE);	//��ͣTimerʱ��ֱ�Ӹ�д
		
	//�Զ���װ��ʹ��
	timer_auto_reload_shadow_enable(TIMER0);
	
    g_stPwmCpsSet.exec      = PWM_CPSSET_EXEC_NONE; //�޴�ִ������
    g_stPwmCpsSet.phase     = PWM_ORG_PHASE_PULSE;  //�������ź�ͬ
    g_stPwmCpsSet.polarity  = PWM_CPSSET_POL_SAME;  //�������ź�ͬ��
		
    nvic_irq_enable(TIMER0_UP_TIMER9_IRQn, 0, 2);
		
	//��ʱ��ʹ��
	timer_enable(TIMER0);	
}

/************************************************************
*	�������ƣ�	Timer0_SetCpsPhase
*	�������ܣ�	Timer���ݳ�ʼ��λ���ò�����λ
*	��ڲ�����	phase	- �����õ���λ(�Գ�ʼ��λ�����ֵ)����Բ�ܰ�632�ȷּ���, ȡֵ��Χ��0~631(��PWM_PERIOD)����ǰ����
*	���ز�����	None
*	˵����	    �ǶȾ���Ϊ0.35��
*				�ֶȷ�ΧΪ0~631(��PWM_FREQ_DIV_NUM*2-1)
*				�ֶ����룺SYNC_HIGH 256             ��0/632��            573
*                         ��___________________________��_________________��    
*               Timer ��0                          256               315  
*               �� �� ��316                        572               631
*				�������룺SYNC_LOW 572   			 316	    	   257
*               
*                           ԭʼ��λ    Ŀ����λ        ͬ��          ����
*               Timer��    1~315       0~315           ��          ��0����
*                              0         0~315         ��0����         ��

*	ע�⣺
*				���㲹����λʱ��Ӧ�Բ����źŵĳ�ʼ��λ��Ϊ�ο�
*				CPS��λ = DRV��λ-CPS��ʼ��λ  ������Ǹ���λ��Ӧ��һ�����ڣ�
*				
*				WNOTE�����ݲ��Խ�����������иı���Ч���ԣ������ܷ�ת�����λ��
*				WNOTE�����ǵ�Timer��д��һ����ֵ�����ʱ������һ�αȽϣ���˲���0�ɷ�ת��λ��
*						ʱ�������ز�������¼�����������0��
*						ʱ���½��ؽ��г���Ƚϣ�
*						�������Ӱ�ӼĴ������£�����һʱ������������һ�αȽϡ�
************************************************************/
void Timer0_SetCpsPhase(uint16_t phase)
{
	uint32_t iPulse;
	uint8_t polarity;
	uint8_t state;
	
	cprintf("   Input phase: %d\n", phase);
	if(phase>(PWM_PERIOD-1))
		phase %= PWM_PERIOD;		
	
	//����������
	if( phase>ANGLE_SYNC_HIGH && phase<=ANGLE_SYNC_LOW)	//�ڵ�ǰ��Χ�⣬��Ҫ���࣬������λ����
	{
		
		iPulse = ANGLE_SYNC_LOW - phase;
		polarity = PWM_CPSSET_POL_ANTI;	    //���Է�
	}
	else	//��Χ�ڣ����÷��࣬Ϊ��������λ����
	{
		if(phase<=ANGLE_SYNC_HIGH)	//����ͬ����
			iPulse = PWM_ORG_PHASE_PULSE - phase; 			//��λԽ����ͬ����ԽԶ�����õ�Timer��
		else							//����ͬ����
			iPulse = (PWM_PERIOD + PWM_ORG_PHASE_PULSE) - phase;
		polarity = PWM_CPSSET_POL_SAME;	    //����Ϊͬ
	}	
	
	if((g_stPwmCpsSet.phase == iPulse) && (g_stPwmCpsSet.polarity == polarity)) //������λ����ԭ������ͬ��������������
        return;
	
    while(g_stPwmCpsSet.exec != PWM_CPSSET_EXEC_NONE);      //�ڵ�ǰ������������ִ�е�ʱ����Ҫ�ȴ���
	
	state = 0;
	if(g_stPwmCpsSet.phase != 0)
		state |= 0x2;	
	if(g_stPwmCpsSet.polarity != polarity)
		state |= 0x1;
	
	g_stPwmCpsSet.phase = iPulse;	
	g_stPwmCpsSet.polarity = polarity;		
	
	if(state==0x3 || state==0x0)	//ԭʼ��λ��Ϊ0�����ҷ���; �� ԭʼ��λΪ0������ͬ��; 
	{
		g_stPwmCpsSet.exec = PWM_CPSSET_EXEC_REVERSE;	//��Ҫ����0���ٸ�
		iPulse = 0;
	}
	else
		g_stPwmCpsSet.exec = PWM_CPSSET_EXEC_REVISE;	//��ֱ���޸�	

    timer_update_event_disable(TIMER0); //��ֹ����; ����ʱ����ȷ������˽�ֹ�����¼�
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, iPulse);	//���ò����������λ  
	if( g_stPwmCpsSet.exec != PWM_CPSSET_EXEC_NONE)	//��Ҫ�ȴ��ж���ɵ�ǰ����
	{
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);  //���ж�
		timer_interrupt_enable(TIMER0, TIMER_INT_UP);	
	}
    timer_update_event_enable(TIMER0);  //��������¼�
	
	cprintf("   Pulse Set: %d\n", iPulse);
}

/************************************************************
*	�������ƣ�	Timer0_GetCpsPhase
*	�������ܣ�	��ȡTimer0��ǰ���õ���λ
*	��ڲ�����	None
*	���ز�����	��ǰ��λ����ǰΪ������Χ��0~631
*	˵����	    
************************************************************/
uint16_t Timer0_GetCpsPhase(void)
{
	int16_t phase;
	phase = PWM_ORG_PHASE_PULSE - (int16_t)g_stPwmCpsSet.phase;	//����λ��Ϊ��ǰΪ��
	if(g_stPwmCpsSet.polarity == PWM_CPSSET_POL_ANTI)	
		phase += PWM_FREQ_DIV_NUM;		//����180��
	if(phase < 0)				
		phase += PWM_PERIOD;			//����360��
	return (uint16_t) phase;
}

#define TIMER_OUTCLK_NUM		(64)	//8bytes*8bit=64CLK
#define TIMER_ADC_FREQ_SCALE	(16)		//84MHz/10.5MHz*2=16	//���Ƶ�ʼ���
#define TIMER_CS_PERIOD	(TIMER_OUTCLK_NUM*TIMER_ADC_FREQ_SCALE)	//�ⲿADCʹ��10.5MHzSCLK��һ�ν���6bytes����\CS��Ҫ�����ļ�������Ϊ��84MHz/10.5MHz*8bits*8bytes=512CLK
#define TIMER_CS_DELAY		(4)		//CS�ӳ�4��ʱ�����ڲ���
#define TIMER_CS_SETUP		(8)		//����ʱ��8����:95ns������GD32��74ns
/************************************************************
*	�������ƣ�	Timer2_Init
*	�������ܣ�	Timer2��ʼ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    Timer2�����ڵ��δ���ģʽ�����ڽ���ADS131M��\DRDY�����źţ�������Ƭѡ�ź�\CS��
*				TIMER2_CH0 Pin63 PC6 ���� /DRDY
*				TIMER2_CH3 Pin66 PC9 ���� /CS
*				����CSΪ����Ч����Timer3ֻ��ʹ���źŸߵ�ƽʱ��������ʹ��TIMER2_CH2��ΪTimer��ʹ���źţ�CH2��CH3��ƽ�෴
*				
*				Timer2ʹ��PCLK1 = AHB/4, ����MULT2����Timer_Clk = AHB/2��84MHz 
*
*				WNOTE�����δ���ʱ������д��CENʹ�ܼ�������
*					   �ڴ����źŵ�������д��CEN=1ʱ�����Զ�����һ�����
*					   �ڼ����������CEN=0ʱ����ֹͣ���������
************************************************************/
void  Timer2_Init(void)
{
	timer_oc_parameter_struct timer_ocintpara;
	timer_ic_parameter_struct timer_icintpara;
    timer_parameter_struct timer_initpara;
	
	//��Timer \CS(PC9)��SCLK(PD14)��\DRDY(PC6)����ΪGPIO����
	rcu_periph_clock_enable(RCU_GPIOC);//����GPIOC��ʱ��
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_9);	//DRDY��CS����Ϊ������GPIO����
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);		//����CS�������ʽ
	gpio_af_set(GPIOC, GPIO_AF_2, GPIO_PIN_6 | GPIO_PIN_9);                          	//����DRDY��CS���Ÿ��ù���,����ΪGPIO_AF_2����ʱ��2/3	
	
	rcu_periph_clock_enable(RCU_GPIOD);//����GPIOD��ʱ��
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);				//SCLK����Ϊ������GPIO����	
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);		//�������������Ƶ��50Mhz
	gpio_af_set(GPIOD, GPIO_AF_2, GPIO_PIN_14);                          				//����SCLK���Ÿ��ù���,����ΪGPIO_AF_2����ʱ��2/3
	
	//ʹ��Timer2ģ��
    rcu_periph_clock_enable(RCU_TIMER2);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER2);
	
	timer_initpara.prescaler         = 0;  					//��ʱ����Ԥ��Ƶ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//���ض���ģʽ
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//���ϼ���ģʽ
    timer_initpara.period            = TIMER_CS_PERIOD+TIMER_CS_DELAY+TIMER_CS_SETUP-1;	//�Զ���װ��ֵ,Ƭѡ384ʱ�����ڣ��ӳ�4ʱ�����ڣ�CS����ʱ��4����
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;	//ʱ�ӷ�Ƶϵ��
	timer_initpara.repetitioncounter = 0; 					//�����ظ�������ֵ
    timer_init(TIMER2,&timer_initpara);	
	
	timer_auto_reload_shadow_enable(TIMER2);//�Զ���װ��ʹ��
	
	//CH3���Ƭѡ�ź�\CS
	timer_channel_output_struct_para_init( &timer_ocintpara);	//��ʼ���������
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;		//�������: ����Ч
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;			//�Ƚ����ʹ��
	timer_channel_output_config(TIMER2,TIMER_CH_3,&timer_ocintpara);				//������õ�CH3
	timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_3, TIMER_CS_DELAY);		//�ӳ�ָ��ʱ����ź���Ч
	timer_channel_output_mode_config(TIMER2,TIMER_CH_3, TIMER_OC_MODE_PWM1);		//������PWM1ģʽ�����ڱȽ�ֵ�����Ч
	timer_channel_output_shadow_config(TIMER2,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);	//��ʹ�ܱȽ�Ӱ�ӼĴ��������й��̲��޸ıȽ�ֵ��	
	
	//����CPU��Slave SPIҪ��CS����ʱ��4���ڣ�ʹ��5���ڣ�������ʱ��2���ڣ�ʹ��3���ڣ�������ʹ��CH2����
	//������ʽֻ�ʺ���STM32������GD32���䱣��ʱ��ʵ�ʸ��󣬿��Կ��ǽ�SPIʱ��Ƶ�ʸ�Ϊ5MHz��������ӳ�
	//������ʱ�����ӵ�10����(8+2)������ʱ��6���ڣ�71ns������GD32��37ns
	timer_channel_output_config(TIMER2,TIMER_CH_2,&timer_ocintpara);				//������õ�CH2
	timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_2, TIMER_CS_DELAY+TIMER_CS_SETUP);		//�ӳ�+����ʱ���ʹ��ʱ���ź���Ч
	timer_channel_output_mode_config(TIMER2,TIMER_CH_2, TIMER_OC_MODE_PWM1);		//������PWM1ģʽ�����ڱȽ�ֵ�����Ч
	timer_channel_output_shadow_config(TIMER2,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);	//��ʹ�ܱȽ�Ӱ�ӼĴ��������й��̲��޸ıȽ�ֵ��	
	
	//CH0����׼�����ź�\DRDY
	timer_icintpara.icpolarity = TIMER_IC_POLARITY_FALLING;		//�����½�����Ч, ͬʱ��תCI0FE0
	timer_icintpara.icselection = TIMER_IC_SELECTION_DIRECTTI;	//���ΪCI0FE0
	timer_icintpara.icprescaler = TIMER_IC_PSC_DIV1;			//����Ƶ
	timer_icintpara.icfilter = 2;								//�ⲿADCʹ��8.4MHz����Ƶ��,������Ϊ4/8.4MHz = 476ns, �˲�����95ns = 4/42MHz
	timer_input_capture_config(TIMER2, TIMER_CH_0, &timer_icintpara);				//�������õ�CH0
	timer_input_trigger_source_select(TIMER2, TIMER_SMCFG_TRGSEL_CI0FE0);			//ʹ��CI0FE0��Ϊ��������	
	
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_PAUSE);						//�Ƚ�����ͣģʽ�����ⴥ���źŴ���������������ź�	
	
	timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_O2CPRE);	//ʹ��CH2 \CS��Ϊ�������	
	
	timer_single_pulse_mode_config(TIMER2, TIMER_SP_MODE_SINGLE);					//���δ���ģʽ
	
	//timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_EVENT);						//�¼��������ڴ�����������������������Ҫʱ����	
    	
    //timer_enable(TIMER2);//��ʱ��ʹ�ܣ�������ʱ�䴥��ģʽʱ������Ҫʹ��ʱ��
}

/************************************************************
*	�������ƣ�	Timer3_Init
*	�������ܣ�	Timer3��ʼ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    Timer3�������ⲿʹ��ģʽ�����ڽ���Timer2������\CS�źţ���ʹ��ʱ������������6bytes*3��ʱ������
*				Timer3���շ�תģʽ���м���
*				TIMER3_CH2 Pin61	���� SCLK���
*				TIMER3_CH3 Pin62	GPIO���� RESET
*				
*				Timer3ʹ��PCLK1 = AHB/4, ����MULT2����Timer_Clk = AHB/2��84MHz 
*
*				WNOTE: CPU��Slave SPI��CS�Ľ����ͱ���ʱ��Ҫ��ϸߣ�STM32�ֱ�Ϊ4����/2���ڣ�GD32Ϊ74ns��37ns
*					   ����ʵ�⣬����STM32����Ϊ5����/3����ʱ��ż������ִ�����˽�SPI SCLKƵ�ʽ���Ϊ5MHz
*					   ����߿ɿ��ԣ�ͬʱ����Timerʹ��CS����ʱ������Ϊ10���ڣ�119ns������ʱ��Ϊ6���ڣ�71ns
************************************************************/
void  Timer3_Init(void)
{
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER3);
	
	timer_initpara.prescaler         = 0;  						//��ʱ����Ԥ��Ƶ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;		//���ñ��ؼ���ģʽ
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;		//���ϼ���ģʽ
    timer_initpara.period            = (TIMER_ADC_FREQ_SCALE>>1)-1;	//�Զ���װ��ֵΪ3
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;		//�޷�Ƶ��ʱ�ӷ�Ƶϵ��
	timer_initpara.repetitioncounter = 0; 						//δ�ã������ظ�������ֵ
    timer_init(TIMER3,&timer_initpara);	
	
	//CH2���ʱ���ź�CLK
	timer_channel_output_struct_para_init( &timer_ocintpara);	//��ʼ���������
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;		//�������: ����Ч
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;			//�Ƚ����ʹ��
	timer_channel_output_config(TIMER3,TIMER_CH_2,&timer_ocintpara);				//������õ�CH2
	
	timer_channel_output_pulse_value_config(TIMER3,TIMER_CH_2, 2);					//�Ƚ�ֵΪ2, clock��������CS�ӳ���8+2=10���������ڣ�CS��������Ϊ8-2=6����
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_LOW);			//��CH2�ź�ǿ���õ�
	timer_channel_output_shadow_config(TIMER3,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);	//��ʹ�ܱȽ�Ӱ�ӼĴ��������й��̲��޸ıȽ�ֵ��	
	
	//Timer3�Ŀ���ģʽ��Slave���ⴥ��
	timer_input_trigger_source_select(TIMER3, TIMER_SMCFG_TRGSEL_ITI2);				//ʹ��Timer2��_TRGO ��Ϊ��������	
	timer_slave_mode_select(TIMER3, TIMER_SLAVE_MODE_PAUSE);						//ֹͣģʽ���ڴ����ź���Чʱ��������ͣģʽʱ������ʹ��Timer�Ż���������Чʱ������
	 
	timer_auto_reload_shadow_enable(TIMER3);//�Զ���װ��ʹ�� 
	 
	//timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);		//�����ڷ�תģʽ�����ڱȽ�ֵʱ��ת������Ҫʱ��Ϊ��תģʽ	 
    //timer_enable(TIMER3);//��ʱ��ʹ�ܣ�����Ҫ��ʱ��ʹ��
}

/************************************************************
*	�������ƣ�	EXADC_Timer_Start
*	�������ܣ�	���������ⲿADC��Timer2��Timer3
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    ����Timerʱ��Ĭ��Timer��SPI�Ѿ���ɳ�ʼ��
*				���Pin��Ӧ��SPI��ʼ���ж�����������������������÷�ʽ��������
*   			TIMER          	SPI				Timer&SPI 
*               /CS		SCLK	/CS		MOSI	/DRDY
*				PC9		PD14	PB12	PB15	PC6
*				T2CH3	T3CH2	GPIO	GPIO	T2CH0/GPIO
*				������Timerģʽʱ��Timer��/CS��SCLK��/DRDY����ΪTimer���ܣ�SPI��/CS����ΪSPI���ܣ�MOSI����ΪGPIO���
*				������SPI��ģʽʱ��Timer��/CS��SCLK��/DRDY����ΪGPIO���룬SPI��/CS����ΪGPIO�����MOSI����ΪSPI����
*
*				SPI1��Timerģʽʱ��Ӧ������
************************************************************/
void EXADC_Timer_Start(void)
{
	//��Timer��\CS��\DRDY��SCLK��Ϊ��ѡ���ܣ�Ӧ�ڴ�ǰ��ʼ���г�ʼ��ΪĬ��Timer����
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_9);	//Timer /CS /DRDY��ΪTimer����,����
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);			//Timer SCLK��ΪTimer����, ����
	
	//Timer3���շ�תģʽ��ʼ����
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);	//��Timer3 CH2�����ڷ�תģʽ�����ڱȽ�ֵʱ��ת������Ҫʱ��Ϊ��תģʽ	 
	timer_enable(TIMER3);	//ʹ��Timer3
}

/************************************************************
*	�������ƣ�	EXADC_TimerDrdyEnable
*	�������ܣ�	��������DRDY����Timer
*	��ڲ�����	None
*	���ز�����	None
*	˵����		���ú�����������Ϊ����ADCͬ��֮�󣬾����õ���һ������
************************************************************/
void EXADC_TimerDrdyEnable(void)
{
	//Timer2���մ������μ���ģʽ��ʼ����
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_EVENT);	//��Timer2��Ϊ�¼���������	
}

/************************************************************
*	�������ƣ�	EXADC_Timer_Stop
*	�������ܣ�	ֹͣ�����ⲿADC��Timer2��Timer3
*	��ڲ�����	None
*	���ز�����	None
*	˵����
*   			TIMER          	SPI				Timer&SPI 
*               /CS		SCLK	/CS		MOSI	/DRDY
*				PC9		PD14	PB12	PB15	PC6
*				T2CH3	T3CH2	GPIO	GPIO	T2CH0/GPIO
*				������Timerģʽʱ��Timer��/CS��SCLK��/DRDY����ΪTimer���ܣ�SPI��/CS����ΪSPI���ܣ�MOSI����ΪGPIO���
*				������SPI��ģʽʱ��Timer��/CS��SCLK��/DRDY����ΪGPIO���룬SPI��/CS����ΪGPIO�����MOSI����ΪSPI����
************************************************************/
void EXADC_Timer_Stop(void)
{
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_PAUSE);	//��Timer2��Ϊ��ͣ
	TIMER_SWEVG(TIMER2) |= (uint32_t)TIMER_SWEVG_UPG;			//��Timer2������0
	
	timer_disable(TIMER3);	//����Timer3
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_LOW);	//��Timer3 CH2ǿ��Ϊ�͵�ƽ���
	
	//��Timer��/CS��SCLK��/DRDY��Ϊ GPIO����
	gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_9);	//Timer /CS��/DRDY��ΪGPIO���롢����
	gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);			//Timer SCLK��ΪGPIO���롢����
}

/************************************************************
*	�������ƣ�	Timer4_Init
*	�������ܣ�	��ʼ������ Timer4 CH0���������ڲ�ADC�ṩ����ת���¼�
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    Timer4ʹ��PCLK1 = AHB/4, ����MULT2����Timer_Clk = AHB/2��84MHz 
*				������1���������arr=1000 psc=42000
************************************************************/
void Timer4_Init(void)
{
	unsigned short arr = 1000u-1u;	//����ֵ
	unsigned short psc = 42000u-1u;	//Ԥ��Ƶ
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER4);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER4);
	
	timer_initpara.prescaler         = psc;  	//��ʱ����Ƶ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//���ض���ģʽ
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//���ϼ���ģʽ
    timer_initpara.period            = arr; 	//�Զ���װ��ֵ
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;//ʱ�ӷ�Ƶϵ��
	timer_initpara.repetitioncounter = 0; //�����ظ�������ֵ
    timer_init(TIMER4,&timer_initpara);

    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;//�������:TIM����Ƚϼ��Ե�
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;//�Ƚ����ʹ��
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER4,TIMER_CH_0,&timer_ocintpara);
	
    timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_0,arr>>1);//������1��ʱ��ת
	
    timer_channel_output_mode_config(TIMER4,TIMER_CH_0,TIMER_OC_MODE_TOGGLE);//�����ڷ�תģʽ
	
    timer_channel_output_shadow_config(TIMER4,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER4);//�Զ���װ��ʹ��
	
    //timer_enable(TIMER4);//��ʱ��ʹ��
}

/************************************************************
*	�������ƣ�	Timer4_SetPeriod
*	�������ܣ�	�ı�Timer4���Զ�����ֵ���޸Ĳ�������
*	��ڲ�����	period - ���ڣ�ms��
*	���ز�����	None
*	˵����	    ����psc���ã�psc=42000����Timerʱ��Ϊ2kHz�����ڹ����ڷ�תģʽ����ô��Ӧms��Ϊ������ֵ
************************************************************/
void Timer4_SetPeriod(unsigned short period)
{
	timer_autoreload_value_config(TIMER4,period);
}

/************************************************************
*	�������ƣ�	Timer7_Init
*	�������ܣ�	��ʼ������ Timer7 CH1 PC7���������ⲿADC�ṩʱ���ź�
*	��ڲ�����	
*	���ز�����	None
*	˵����	    Timer7ʹ��PCLK2 = AHB/2, ����MUL2����Timer_Clk = AHB��168MHz
*				������ʱ��Ϊ168MHz����������ʱ��ֻ��Ϊ8.4MHz��ǡ��ΪADS131M02������Ƶ�ʡ�
*
*				ע��Ҫʹ�ø����ܵ�ʱ�ӣ���Ҫʹ��I2Sʱ�������ͬʱӦ����PLL
*				PLL��   ��8MHz/5=1.6MHz; ��1.6MHz*210=336MHz����336MHz/2=168MHz��
*				PLLI2S����8MHz/5=1.6MHz; ��1.6MHz*256=409.6MHz����409.6Mhz/5=81.92MHz����81.92MHz/10=8.192MHz
************************************************************/
void Timer7_Init(void)
{
	unsigned short psc = 2u-1u;		//Ԥ��Ƶ��Ϊ��168MHz/2��84MHz
	unsigned short arr = 5u-1u;		//���ز����շ�תģʽ��84MHz/10��8.4MHz
	
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable(RCU_GPIOC);//ʹ��GPIOCʱ��

    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);//���ù�������PC7(TIMER7_CH1)
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7);

    gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_7);
	
    rcu_periph_clock_enable(RCU_TIMER7);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);

    timer_deinit(TIMER7);
	
	timer_primary_output_config(TIMER7, ENABLE);	//ʹ��������߼��жϱ���ʹ�ܸüĴ����������
	
	timer_initpara.prescaler         = psc;  	//��ʱ����Ƶ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//���ض���ģʽ
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//���ϼ���ģʽ
    timer_initpara.period            = arr; 	//�Զ���װ��ֵ
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;//ʱ�ӷ�Ƶϵ��
	timer_initpara.repetitioncounter = 0; //�����ظ�������ֵ
    timer_init(TIMER7,&timer_initpara);

    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;//�������:TIM����Ƚϼ��Ե�
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;//�Ƚ����ʹ��
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER7,TIMER_CH_1,&timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,arr>>1);//��һ�뿪ʼ��ת
	
    timer_channel_output_mode_config(TIMER7,TIMER_CH_1,TIMER_OC_MODE_TOGGLE);//��תģʽ���Ա�õ����õ�ռ�ձ�
	
    timer_channel_output_shadow_config(TIMER7,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER7);//�Զ���װ��ʹ��
	
    timer_enable(TIMER7);//��ʱ��ʹ��
}

/************************************************************
*	�������ƣ�	TIMER0_UP_TIMER9_IRQHandler
*	�������ܣ�	��Timer0�����жϵ���ʱ������������λ
*	��ڲ�����	None
*	���ز�����	None
*	˵����	    
************************************************************/
void TIMER0_UP_TIMER9_IRQHandler(void)
{
    if( SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP))    //����Timer0�����¼�
    {     
       timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, g_stPwmCpsSet.phase);	//���ò����������λ 
	   g_stPwmCpsSet.exec >>= 2;
       timer_interrupt_disable(TIMER0, TIMER_INT_UP);
       timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);  //���ж�
    }
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

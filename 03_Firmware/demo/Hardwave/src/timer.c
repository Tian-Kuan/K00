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
** Descriptions:            1. 修改Timer2、Timer3用途
**							名称	改后用途				改前用途
**							Timer0	同改前					用于线圈驱动用正弦信号的产生、补偿及解调
**							Timer2  接收ADC DRDY, 发出CS	用于CPU内部ADC的的转换中断频率计时
**							Timer3  根据CS, 发出SCLK		用于对ADC片选信号的计时复位
**							Timer4  同改前的Timer2			无
**							Timer7  同改前					用于向外部ADC提供主时钟					
**
**							2. 原为266kHz频率，增加对466、699、932kHz的支持
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
    uint16_t phase;     //待设置相位/当前相位
    uint8_t  polarity;  //待设置极性/当前极性，0：与驱动同，1：反相
    uint8_t  exec;      //执行内容，每2bit代表一步，低2bit开始：00 - 直接执行； 10 - 需翻转
} tPWMCpsSet;


#define PWM_CPSSET_POL_ANTI    (1)    //反相
#define PWM_CPSSET_POL_SAME    (0)    //同相

#define PWM_CPSSET_EXEC_NONE     (0x0)   //无待执行内容
//下述定义需要等待0~1个中断，不完全结束；但直接修改内容可被其他修改操作覆盖
#define PWM_CPSSET_EXEC_REVISE 	 (PWM_CPSSET_EXEC_NONE)			//直接修改时，不用等待；可被其他修改插入改掉
#define PWM_CPSSET_EXEC_REVERSE  (0x2)							//插0翻转，等待一个中断

/** ---	Start of Para define 	---------------------------------------------------------------------  **/
static tPWMCpsSet g_stPwmCpsSet;

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
/************************************************************
*	函数名称：	Timer0_Init
*	函数功能：	初始化配置 Timer0，用于模拟信号产生、补偿及解调
*	入口参数：	
*	返回参数：	None
*	说明：	    PE09 - PWM_D    CH0
*               PE10 - DEM_I-   CH1（Sin，滞后90°）
*               PE11 - DEM_I+   
*               PE12 - DEM_Q-   CH2（Cos，同CH0相位）
*               PE13 - DEM_Q+
*               PE14 - PWM_C    CH3
*				Timer0使用PCLK2 = AHB/2, 设置MUL2，故Timer_Clk = AHB：168MHz 
************************************************************/
void Timer0_Init(void)
{
	timer_parameter_struct timer_initpara;	
	timer_oc_parameter_struct timer_ocintpara;	
	
    //--------------GPIO配置--------------
	rcu_periph_clock_enable(RCU_GPIOE);     	            //使能GPIOE时钟	
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE,      //备用功能配置PE9~14(TIMER0_CH0)  PE9~14复用、浮空
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
	gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,    	//设置io模式
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
	gpio_af_set(GPIOE, GPIO_AF_1,                           //设置引脚复用功能,这里设置为GPIO_AF_1即定时器0
            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);

    //---------------定时器配置---------------	
	rcu_periph_clock_enable(RCU_TIMER0);    //使能定时器时钟
		
	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//由于PCLK2为AHB/2，MUL2后Timer时钟为AHB: 168MHz
	
	timer_deinit(TIMER0);   //复位定时器	
	
	timer_primary_output_config(TIMER0, ENABLE);	//使能互补输出
		
	timer_initpara.prescaler         = PWM_PSC-1;           //定时器分频	
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//边沿对齐模式
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;    //向上计数模式
	timer_initpara.period            = PWM_FREQ_DIV_NUM-1;  //自动重装载值	
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;    //时钟分频系数	
	timer_initpara.repetitioncounter = 0;                   //设置重复计数器值	
	timer_init(TIMER0,&timer_initpara);
		
    //----------------初始化PWM通道-----------------	
    //CH1&CH2 DEM_I/DEM_Q 互补输出，WNOTE：互补通道与主通道之间空闲与有效是交替的，即主通道空闲，则互补有效
	//WNOTE: 开关TMUX6111为0开1关，因此注意输出应与PWM_D、PWM_C相反；而同时DEM_I应该滞后DEM_Q90°，才能得到正确的Sin/Cos相位
	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;        //比较输出使能	
	timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;       //是否互补通道输出	
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;	//输出有效极性: 低
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_LOW;  //互补有效极性：低
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH; //空闲状态下通道输出极性：高
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;//空闲时互补通道输出极性：高
	timer_channel_output_config(TIMER0,TIMER_CH_1,&timer_ocintpara);
	timer_channel_output_config(TIMER0,TIMER_CH_2,&timer_ocintpara); 

    //CH0&CH3 PWM_D/PWM_C 单路比较输出
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;      //不是互补通道输出	
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;	//输出极性: 高	
	timer_channel_output_config(TIMER0,TIMER_CH_0,&timer_ocintpara);
	timer_channel_output_config(TIMER0,TIMER_CH_3,&timer_ocintpara);		

	//设置输出翻转计数值：
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,PWM_ORG_PHASE_PULSE);	//PE9 -->PWM_Dri
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,PWM_ORG_PHASE_PULSE-(PWM_FREQ_DIV_NUM>>1));	//PE11-->Sin_Carr, 滞后Cos90°，由于开关为反控制，实际超前90°
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,PWM_ORG_PHASE_PULSE);	//PE13-->Cos_Carr
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_3,PWM_ORG_PHASE_PULSE);	//PE14-->PWM_Comp
		
	//输出模式配置为翻转模式
	timer_channel_output_mode_config(TIMER0,TIMER_CH_0,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_1,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);
	timer_channel_output_mode_config(TIMER0,TIMER_CH_3,TIMER_OC_MODE_TOGGLE);
		
	//不使用影子寄存器
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_3,TIMER_OC_SHADOW_ENABLE);	//暂停Timer时，直接改写
		
	//自动重装载使能
	timer_auto_reload_shadow_enable(TIMER0);
	
    g_stPwmCpsSet.exec      = PWM_CPSSET_EXEC_NONE; //无待执行内容
    g_stPwmCpsSet.phase     = PWM_ORG_PHASE_PULSE;  //与驱动信号同
    g_stPwmCpsSet.polarity  = PWM_CPSSET_POL_SAME;  //与驱动信号同相
		
    nvic_irq_enable(TIMER0_UP_TIMER9_IRQn, 0, 2);
		
	//定时器使能
	timer_enable(TIMER0);	
}

/************************************************************
*	函数名称：	Timer0_SetCpsPhase
*	函数功能：	Timer根据初始相位设置补偿相位
*	入口参数：	phase	- 待设置的相位(对初始相位的相对值)，对圆周按632等分计数, 取值范围：0~631(即PWM_PERIOD)，超前度数
*	返回参数：	None
*	说明：	    角度精度为0.35°
*				分度范围为0~631(即PWM_FREQ_DIV_NUM*2-1)
*				分度输入：SYNC_HIGH 256             ←0/632→            573
*                         ↑___________________________↑_________________↑    
*               Timer 域：0                          256               315  
*               反 向 后：316                        572               631
*				反向输入：SYNC_LOW 572   			 316	    	   257
*               
*                           原始相位    目标相位        同相          反相
*               Timer域：    1~315       0~315           改          插0，改
*                              0         0~315         插0，改         改

*	注意：
*				计算补偿相位时，应以补偿信号的初始相位作为参考
*				CPS相位 = DRV相位-CPS初始相位  （如果是负相位，应加一个周期）
*				
*				WNOTE：根据测试结果，在运行中改变有效极性，并不能反转输出相位。
*				WNOTE：考虑到Timer在写入一个新值后更新时会增加一次比较，因此插入0可翻转相位。
*						时钟上升沿产生溢出事件，计数器回0；
*						时钟下降沿进行常规比较；
*						如果存在影子寄存器更新，在下一时钟上升沿增加一次比较。
************************************************************/
void Timer0_SetCpsPhase(uint16_t phase)
{
	uint32_t iPulse;
	uint8_t polarity;
	uint8_t state;
	
	cprintf("   Input phase: %d\n", phase);
	if(phase>(PWM_PERIOD-1))
		phase %= PWM_PERIOD;		
	
	//脉冲数设置
	if( phase>ANGLE_SYNC_HIGH && phase<=ANGLE_SYNC_LOW)	//在当前范围外，需要反相，连续相位输入
	{
		
		iPulse = ANGLE_SYNC_LOW - phase;
		polarity = PWM_CPSSET_POL_ANTI;	    //极性反
	}
	else	//范围内，不用反相，为不连续相位输入
	{
		if(phase<=ANGLE_SYNC_HIGH)	//早于同步点
			iPulse = PWM_ORG_PHASE_PULSE - phase; 			//相位越大离同步点越远，设置到Timer域
		else							//晚于同步点
			iPulse = (PWM_PERIOD + PWM_ORG_PHASE_PULSE) - phase;
		polarity = PWM_CPSSET_POL_SAME;	    //极性为同
	}	
	
	if((g_stPwmCpsSet.phase == iPulse) && (g_stPwmCpsSet.polarity == polarity)) //极性相位均与原设置相同，不用重新配置
        return;
	
    while(g_stPwmCpsSet.exec != PWM_CPSSET_EXEC_NONE);      //在当前仍有配置正在执行的时候，需要等待；
	
	state = 0;
	if(g_stPwmCpsSet.phase != 0)
		state |= 0x2;	
	if(g_stPwmCpsSet.polarity != polarity)
		state |= 0x1;
	
	g_stPwmCpsSet.phase = iPulse;	
	g_stPwmCpsSet.polarity = polarity;		
	
	if(state==0x3 || state==0x0)	//原始相位不为0，并且反相; 或 原始相位为0，并且同相; 
	{
		g_stPwmCpsSet.exec = PWM_CPSSET_EXEC_REVERSE;	//需要插入0，再改
		iPulse = 0;
	}
	else
		g_stPwmCpsSet.exec = PWM_CPSSET_EXEC_REVISE;	//可直接修改	

    timer_update_event_disable(TIMER0); //禁止更新; 配置时机不确定，因此禁止更新事件
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, iPulse);	//设置补偿输出的相位  
	if( g_stPwmCpsSet.exec != PWM_CPSSET_EXEC_NONE)	//需要等待中断完成当前操作
	{
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);  //清中断
		timer_interrupt_enable(TIMER0, TIMER_INT_UP);	
	}
    timer_update_event_enable(TIMER0);  //允许更新事件
	
	cprintf("   Pulse Set: %d\n", iPulse);
}

/************************************************************
*	函数名称：	Timer0_GetCpsPhase
*	函数功能：	获取Timer0当前配置的相位
*	入口参数：	None
*	返回参数：	当前相位，超前为正，范围：0~631
*	说明：	    
************************************************************/
uint16_t Timer0_GetCpsPhase(void)
{
	int16_t phase;
	phase = PWM_ORG_PHASE_PULSE - (int16_t)g_stPwmCpsSet.phase;	//将相位改为超前为正
	if(g_stPwmCpsSet.polarity == PWM_CPSSET_POL_ANTI)	
		phase += PWM_FREQ_DIV_NUM;		//增加180°
	if(phase < 0)				
		phase += PWM_PERIOD;			//增加360°
	return (uint16_t) phase;
}

#define TIMER_OUTCLK_NUM		(64)	//8bytes*8bit=64CLK
#define TIMER_ADC_FREQ_SCALE	(16)		//84MHz/10.5MHz*2=16	//输出频率减半
#define TIMER_CS_PERIOD	(TIMER_OUTCLK_NUM*TIMER_ADC_FREQ_SCALE)	//外部ADC使用10.5MHzSCLK，一次接收6bytes，故\CS需要持续的计数周期为：84MHz/10.5MHz*8bits*8bytes=512CLK
#define TIMER_CS_DELAY		(4)		//CS延迟4个时钟周期产生
#define TIMER_CS_SETUP		(8)		//建立时间8周期:95ns，大于GD32的74ns
/************************************************************
*	函数名称：	Timer2_Init
*	函数功能：	Timer2初始化
*	入口参数：	None
*	返回参数：	None
*	说明：	    Timer2工作于单次触发模式，用于接收ADS131M的\DRDY触发信号，并发出片选信号\CS，
*				TIMER2_CH0 Pin63 PC6 连接 /DRDY
*				TIMER2_CH3 Pin66 PC9 连接 /CS
*				由于CS为低有效，而Timer3只在使能信号高电平时计数，故使用TIMER2_CH2作为Timer的使能信号，CH2与CH3电平相反
*				
*				Timer2使用PCLK1 = AHB/4, 设置MULT2，故Timer_Clk = AHB/2：84MHz 
*
*				WNOTE：单次触发时，不用写入CEN使能计数器；
*					   在触发信号到来或者写入CEN=1时，会自动启动一次输出
*					   在计数溢出或者CEN=0时，会停止计数和输出
************************************************************/
void  Timer2_Init(void)
{
	timer_oc_parameter_struct timer_ocintpara;
	timer_ic_parameter_struct timer_icintpara;
    timer_parameter_struct timer_initpara;
	
	//将Timer \CS(PC9)、SCLK(PD14)、\DRDY(PC6)配置为GPIO输入
	rcu_periph_clock_enable(RCU_GPIOC);//开启GPIOC的时钟
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_9);	//DRDY、CS配置为上拉，GPIO输入
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);		//配置CS的输出方式
	gpio_af_set(GPIOC, GPIO_AF_2, GPIO_PIN_6 | GPIO_PIN_9);                          	//设置DRDY、CS引脚复用功能,设置为GPIO_AF_2即定时器2/3	
	
	rcu_periph_clock_enable(RCU_GPIOD);//开启GPIOD的时钟
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);				//SCLK配置为下拉，GPIO输入	
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);		//推挽输出，引脚频率50Mhz
	gpio_af_set(GPIOD, GPIO_AF_2, GPIO_PIN_14);                          				//设置SCLK引脚复用功能,设置为GPIO_AF_2即定时器2/3
	
	//使能Timer2模块
    rcu_periph_clock_enable(RCU_TIMER2);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER2);
	
	timer_initpara.prescaler         = 0;  					//定时器不预分频
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//边沿对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//向上计数模式
    timer_initpara.period            = TIMER_CS_PERIOD+TIMER_CS_DELAY+TIMER_CS_SETUP-1;	//自动重装载值,片选384时钟周期，延迟4时钟周期，CS建立时间4周期
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;	//时钟分频系数
	timer_initpara.repetitioncounter = 0; 					//设置重复计数器值
    timer_init(TIMER2,&timer_initpara);	
	
	timer_auto_reload_shadow_enable(TIMER2);//自动重装载使能
	
	//CH3输出片选信号\CS
	timer_channel_output_struct_para_init( &timer_ocintpara);	//初始化输出参数
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;		//输出极性: 低有效
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;			//比较输出使能
	timer_channel_output_config(TIMER2,TIMER_CH_3,&timer_ocintpara);				//输出配置到CH3
	timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_3, TIMER_CS_DELAY);		//延迟指定时间后，信号有效
	timer_channel_output_mode_config(TIMER2,TIMER_CH_3, TIMER_OC_MODE_PWM1);		//工作于PWM1模式，高于比较值输出有效
	timer_channel_output_shadow_config(TIMER2,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);	//不使能比较影子寄存器（运行过程不修改比较值）	
	
	//由于CPU的Slave SPI要求CS建立时间4周期（使用5周期），保持时间2周期（使用3周期），单独使用CH2控制
	//上述方式只适合于STM32，对于GD32，其保持时间实际更大，可以考虑将SPI时钟频率改为5MHz，以提高延迟
	//将建立时间增加到10周期(8+2)，保持时间6周期：71ns，大于GD32的37ns
	timer_channel_output_config(TIMER2,TIMER_CH_2,&timer_ocintpara);				//输出配置到CH2
	timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_2, TIMER_CS_DELAY+TIMER_CS_SETUP);		//延迟+建立时间后，使能时钟信号有效
	timer_channel_output_mode_config(TIMER2,TIMER_CH_2, TIMER_OC_MODE_PWM1);		//工作于PWM1模式，高于比较值输出有效
	timer_channel_output_shadow_config(TIMER2,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);	//不使能比较影子寄存器（运行过程不修改比较值）	
	
	//CH0输入准备好信号\DRDY
	timer_icintpara.icpolarity = TIMER_IC_POLARITY_FALLING;		//输入下降沿有效, 同时翻转CI0FE0
	timer_icintpara.icselection = TIMER_IC_SELECTION_DIRECTTI;	//输出为CI0FE0
	timer_icintpara.icprescaler = TIMER_IC_PSC_DIV1;			//不分频
	timer_icintpara.icfilter = 2;								//外部ADC使用8.4MHz工作频率,脉冲宽度为4/8.4MHz = 476ns, 滤波按照95ns = 4/42MHz
	timer_input_capture_config(TIMER2, TIMER_CH_0, &timer_icintpara);				//输入配置到CH0
	timer_input_trigger_source_select(TIMER2, TIMER_SMCFG_TRGSEL_CI0FE0);			//使用CI0FE0作为触发输入	
	
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_PAUSE);						//先进入暂停模式，避免触发信号错误到来而输出错误信号	
	
	timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_O2CPRE);	//使用CH2 \CS作为触发输出	
	
	timer_single_pulse_mode_config(TIMER2, TIMER_SP_MODE_SINGLE);					//单次触发模式
	
	//timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_EVENT);						//事件触发，在触发上升沿启动计数，在需要时设置	
    	
    //timer_enable(TIMER2);//定时器使能，工作于时间触发模式时，不需要使能时钟
}

/************************************************************
*	函数名称：	Timer3_Init
*	函数功能：	Timer3初始化
*	入口参数：	None
*	返回参数：	None
*	说明：	    Timer3工作于外部使能模式，用于接收Timer2发出的\CS信号，以使能时钟输出，共输出6bytes*3个时钟脉冲
*				Timer3按照翻转模式进行计数
*				TIMER3_CH2 Pin61	连接 SCLK输出
*				TIMER3_CH3 Pin62	GPIO连接 RESET
*				
*				Timer3使用PCLK1 = AHB/4, 设置MULT2，故Timer_Clk = AHB/2：84MHz 
*
*				WNOTE: CPU的Slave SPI对CS的建立和保持时间要求较高，STM32分别为4周期/2周期，GD32为74ns，37ns
*					   经过实测，按照STM32设置为5周期/3周期时，偶尔会出现错误，因此将SPI SCLK频率降低为5MHz
*					   以提高可靠性，同时调整Timer使得CS建立时间设置为10周期：119ns，保持时间为6周期：71ns
************************************************************/
void  Timer3_Init(void)
{
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER3);
	
	timer_initpara.prescaler         = 0;  						//定时器不预分频
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;		//采用边沿计数模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;		//向上计数模式
    timer_initpara.period            = (TIMER_ADC_FREQ_SCALE>>1)-1;	//自动重装载值为3
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;		//无分频，时钟分频系数
	timer_initpara.repetitioncounter = 0; 						//未用，设置重复计数器值
    timer_init(TIMER3,&timer_initpara);	
	
	//CH2输出时钟信号CLK
	timer_channel_output_struct_para_init( &timer_ocintpara);	//初始化输出参数
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;		//输出极性: 高有效
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;			//比较输出使能
	timer_channel_output_config(TIMER3,TIMER_CH_2,&timer_ocintpara);				//输出配置到CH2
	
	timer_channel_output_pulse_value_config(TIMER3,TIMER_CH_2, 2);					//比较值为2, clock输出相对于CS延迟了8+2=10个建立周期，CS保持周期为8-2=6个。
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_LOW);			//对CH2信号强制置低
	timer_channel_output_shadow_config(TIMER3,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);	//不使能比较影子寄存器（运行过程不修改比较值）	
	
	//Timer3的控制模式：Slave、外触发
	timer_input_trigger_source_select(TIMER3, TIMER_SMCFG_TRGSEL_ITI2);				//使用Timer2—_TRGO 作为触发输入	
	timer_slave_mode_select(TIMER3, TIMER_SLAVE_MODE_PAUSE);						//停止模式，在触发信号有效时计数；暂停模式时，必须使能Timer才会在输入有效时计数。
	 
	timer_auto_reload_shadow_enable(TIMER3);//自动重装载使能 
	 
	//timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);		//工作于翻转模式，等于比较值时翻转，在需要时改为翻转模式	 
    //timer_enable(TIMER3);//定时器使能，在需要的时候使能
}

/************************************************************
*	函数名称：	EXADC_Timer_Start
*	函数功能：	启动用于外部ADC的Timer2、Timer3
*	入口参数：	None
*	返回参数：	None
*	说明：	    启动Timer时，默认Timer和SPI已经完成初始化
*				相关Pin脚应在SPI初始化中对其上下拉、输出参数、复用方式进行配置
*   			TIMER          	SPI				Timer&SPI 
*               /CS		SCLK	/CS		MOSI	/DRDY
*				PC9		PD14	PB12	PB15	PC6
*				T2CH3	T3CH2	GPIO	GPIO	T2CH0/GPIO
*				工作于Timer模式时，Timer的/CS、SCLK、/DRDY设置为Timer功能，SPI的/CS设置为SPI功能，MOSI设置为GPIO输出
*				工作于SPI主模式时，Timer的/CS、SCLK、/DRDY设置为GPIO输入，SPI的/CS设置为GPIO输出，MOSI设置为SPI功能
*
*				SPI1在Timer模式时，应工作在
************************************************************/
void EXADC_Timer_Start(void)
{
	//将Timer的\CS、\DRDY、SCLK改为可选功能，应在此前初始化中初始化为默认Timer功能
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_9);	//Timer /CS /DRDY改为Timer功能,上拉
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);			//Timer SCLK改为Timer功能, 下拉
	
	//Timer3按照翻转模式开始工作
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_TOGGLE);	//令Timer3 CH2工作于翻转模式，等于比较值时翻转，在需要时改为翻转模式	 
	timer_enable(TIMER3);	//使能Timer3
}

/************************************************************
*	函数名称：	EXADC_TimerDrdyEnable
*	函数功能：	用于允许DRDY启动Timer
*	入口参数：	None
*	返回参数：	None
*	说明：		将该函数独立，是为了在ADC同步之后，尽快拿到第一个数据
************************************************************/
void EXADC_TimerDrdyEnable(void)
{
	//Timer2按照触发单次计数模式开始工作
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_EVENT);	//将Timer2改为事件触发计数	
}

/************************************************************
*	函数名称：	EXADC_Timer_Stop
*	函数功能：	停止用于外部ADC的Timer2、Timer3
*	入口参数：	None
*	返回参数：	None
*	说明：
*   			TIMER          	SPI				Timer&SPI 
*               /CS		SCLK	/CS		MOSI	/DRDY
*				PC9		PD14	PB12	PB15	PC6
*				T2CH3	T3CH2	GPIO	GPIO	T2CH0/GPIO
*				工作于Timer模式时，Timer的/CS、SCLK、/DRDY设置为Timer功能，SPI的/CS设置为SPI功能，MOSI设置为GPIO输出
*				工作于SPI主模式时，Timer的/CS、SCLK、/DRDY设置为GPIO输入，SPI的/CS设置为GPIO输出，MOSI设置为SPI功能
************************************************************/
void EXADC_Timer_Stop(void)
{
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_PAUSE);	//将Timer2改为暂停
	TIMER_SWEVG(TIMER2) |= (uint32_t)TIMER_SWEVG_UPG;			//将Timer2计数清0
	
	timer_disable(TIMER3);	//禁用Timer3
	timer_channel_output_mode_config(TIMER3,TIMER_CH_2,TIMER_OC_MODE_LOW);	//将Timer3 CH2强制为低电平输出
	
	//将Timer的/CS、SCLK、/DRDY改为 GPIO输入
	gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_9);	//Timer /CS、/DRDY改为GPIO输入、上拉
	gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);			//Timer SCLK改为GPIO输入、下拉
}

/************************************************************
*	函数名称：	Timer4_Init
*	函数功能：	初始化配置 Timer4 CH0，用于向内部ADC提供启动转换事件
*	入口参数：	None
*	返回参数：	None
*	说明：	    Timer4使用PCLK1 = AHB/4, 设置MULT2，故Timer_Clk = AHB/2：84MHz 
*				若按照1秒输出，则arr=1000 psc=42000
************************************************************/
void Timer4_Init(void)
{
	unsigned short arr = 1000u-1u;	//重载值
	unsigned short psc = 42000u-1u;	//预分频
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER4);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);//Timer_Clk=84MHz

    timer_deinit(TIMER4);
	
	timer_initpara.prescaler         = psc;  	//定时器分频
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//边沿对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//向上计数模式
    timer_initpara.period            = arr; 	//自动重装载值
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;//时钟分频系数
	timer_initpara.repetitioncounter = 0; //设置重复计数器值
    timer_init(TIMER4,&timer_initpara);

    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;//输出极性:TIM输出比较极性低
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;//比较输出使能
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER4,TIMER_CH_0,&timer_ocintpara);
	
    timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_0,arr>>1);//计数到1半时翻转
	
    timer_channel_output_mode_config(TIMER4,TIMER_CH_0,TIMER_OC_MODE_TOGGLE);//工作在翻转模式
	
    timer_channel_output_shadow_config(TIMER4,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER4);//自动重装载使能
	
    //timer_enable(TIMER4);//定时器使能
}

/************************************************************
*	函数名称：	Timer4_SetPeriod
*	函数功能：	改变Timer4的自动重载值，修改产生周期
*	入口参数：	period - 周期（ms）
*	返回参数：	None
*	说明：	    按照psc设置：psc=42000，则Timer时钟为2kHz，由于工作在翻转模式，那么对应ms即为重载数值
************************************************************/
void Timer4_SetPeriod(unsigned short period)
{
	timer_autoreload_value_config(TIMER4,period);
}

/************************************************************
*	函数名称：	Timer7_Init
*	函数功能：	初始化配置 Timer7 CH1 PC7，用于向外部ADC提供时钟信号
*	入口参数：	
*	返回参数：	None
*	说明：	    Timer7使用PCLK2 = AHB/2, 设置MUL2，故Timer_Clk = AHB：168MHz
*				由于主时钟为168MHz，因此输出的时钟只能为8.4MHz，恰好为ADS131M02的上限频率。
*
*				注：要使用更精密的时钟，需要使用I2S时钟输出，同时应调整PLL
*				PLL：   ①8MHz/5=1.6MHz; ②1.6MHz*210=336MHz；③336MHz/2=168MHz；
*				PLLI2S：①8MHz/5=1.6MHz; ②1.6MHz*256=409.6MHz；③409.6Mhz/5=81.92MHz；④81.92MHz/10=8.192MHz
************************************************************/
void Timer7_Init(void)
{
	unsigned short psc = 2u-1u;		//预分频后为：168MHz/2≈84MHz
	unsigned short arr = 5u-1u;		//重载并按照翻转模式后：84MHz/10≈8.4MHz
	
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable(RCU_GPIOC);//使能GPIOC时钟

    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);//备用功能配置PC7(TIMER7_CH1)
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7);

    gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_7);
	
    rcu_periph_clock_enable(RCU_TIMER7);
	
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2);

    timer_deinit(TIMER7);
	
	timer_primary_output_config(TIMER7, ENABLE);	//使能输出，高级中断必须使能该寄存器才能输出
	
	timer_initpara.prescaler         = psc;  	//定时器分频
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;	//边沿对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;	//向上计数模式
    timer_initpara.period            = arr; 	//自动重装载值
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;//时钟分频系数
	timer_initpara.repetitioncounter = 0; //设置重复计数器值
    timer_init(TIMER7,&timer_initpara);

    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;//输出极性:TIM输出比较极性低
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;//比较输出使能
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER7,TIMER_CH_1,&timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,arr>>1);//从一半开始翻转
	
    timer_channel_output_mode_config(TIMER7,TIMER_CH_1,TIMER_OC_MODE_TOGGLE);//翻转模式，以便得到更好的占空比
	
    timer_channel_output_shadow_config(TIMER7,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER7);//自动重装载使能
	
    timer_enable(TIMER7);//定时器使能
}

/************************************************************
*	函数名称：	TIMER0_UP_TIMER9_IRQHandler
*	函数功能：	在Timer0更新中断到来时，更新最新相位
*	入口参数：	None
*	返回参数：	None
*	说明：	    
************************************************************/
void TIMER0_UP_TIMER9_IRQHandler(void)
{
    if( SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP))    //发生Timer0更新事件
    {     
       timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, g_stPwmCpsSet.phase);	//设置补偿输出的相位 
	   g_stPwmCpsSet.exec >>= 2;
       timer_interrupt_disable(TIMER0, TIMER_INT_UP);
       timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);  //清中断
    }
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

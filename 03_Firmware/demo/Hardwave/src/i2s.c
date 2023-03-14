/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               I2S.c
** Latest modified Date:    2022-5-28
** Latest Version:          1.0
** Descriptions:            To Generate ADC 8.192MHz Clok
**                          Use I2S Mck output, and need to adjust the PLL
*				            PLL：   ①8MHz/5=1.6MHz; ②1.6MHz*210=336MHz；③336MHz/2=168MHz；
*				            PLLI2S：①8MHz/5=1.6MHz; ②1.6MHz*256=409.6MHz；③409.6Mhz/5=81.92MHz；④81.92MHz/10=8.192MHz
**							
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-28
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
#include "gd32f4xx.h"
#include "systick.h"

#include "i2s.h"
#include "console.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/
//#define PLL_CFG_8192		//(需要修改主PLL时钟)

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/

/************************************************************
*	函数名称：	I2S_PllSwitch
*	函数功能：	为适应I2S时钟，切换PLL工作时钟
*	入口参数：	None
*	返回参数：	None
*	说明：      PLL的要求：输入1~4MHz；输出：100~500MHz；
*				为得到8.192MHz，要求配置为：
*				PLL：   ①8MHz/5=1.6MHz; ②1.6MHz*210=336MHz；③336MHz/2=168MHz；
*				PLLI2S：①8MHz/5=1.6MHz; ②1.6MHz*256=409.6MHz；③409.6Mhz/5=81.92MHz；④81.92MHz/10=8.192MHz
*				默认的配置输出为8MHz：
*				PLL：	①8MHz/8=1MHz; ②1MHz*336=336MHz；③336MHz/2=168MHz；
*				PLLI2S: ①8MHz/8=1MHz; ②1MHz*256=256MHz；③256MHz/4=64MHz；④64MHz/8=8MHz
************************************************************/
void I2SCLK_PllSwitch(void)
{
    //该段考虑改到系统初始化中
#ifdef PLL_CFG_8192	
    rcu_pll_config(RCU_PLLSRC_HXTAL, 5, 210, 2, 7);//用于要求配置	
    rcu_osci_stab_wait(RCU_PLL_CK);    
#endif
    
	//设置I2S PLL
#ifdef PLL_CFG_8192	
    rcu_plli2s_config(256,5);       //要求配置，输出8.192MHz
#else	
	rcu_plli2s_config(256,4);		//默认配置，输出8MHz
#endif
	
    rcu_osci_on(RCU_PLLI2S_CK);     //开启PLL
    rcu_osci_stab_wait(RCU_PLLI2S_CK);  //等待PLL锁定
    
    rcu_i2s_clock_config(RCU_I2SSRC_PLLI2S);    //将I2S时钟配置为PLLI2S输出
}

/************************************************************
*	函数名称：	I2S_Init
*	函数功能：	初始化I2S以产生MCK时钟
*	入口参数：	None
*	返回参数：	None
*	说明：      PC7 - I2S2_MCK
*               I2S2与SPI2为同一模块
*               不可与Timer7_CH1同时使用
************************************************************/
void I2SCLK_Init(void)
{
    I2SCLK_PllSwitch();    
  
	//与Timer复用时，该段在Timer中配置
//	rcu_periph_clock_enable(RCU_GPIOC);//使能GPIOC时钟

//    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);//备用功能配置PC7(I2S2_MCK)
//    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7);
// 
//    gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_7);  //设置为I2S2_MCK
	
    rcu_periph_clock_enable(RCU_SPI2);  //使能模块时钟
    
    spi_i2s_deinit(SPI2);   //模块复位
 
#ifdef PLL_CFG_8192		
    i2s_psc_config(SPI2,I2S_AUDIOSAMPLE_32K,I2S_FRAMEFORMAT_DT16B_CH16B,I2S_MCKOUT_ENABLE); //使能MCLK输出, 并配置为10分频（需求配置8.912MHz输出时）   
#else
    i2s_psc_config(SPI2, 31250u,I2S_FRAMEFORMAT_DT16B_CH16B,I2S_MCKOUT_ENABLE);	//默认配置8MHz输出时，配置为8分频
#endif
	
    i2s_init(SPI2, I2S_MODE_MASTERRX, I2S_STD_PHILLIPS, I2S_CKPL_LOW);  //模块初始化为I2S主收模式
   
//    i2s_enable(SPI2);   //使能输出
}

void I2SCLK_Test(void)
{
	cprintf("\nTest Switch ADS131M02 CLK to I2S 8MHz CLK(Old Timer CLK: 8.4MHz)\n");
	//停止TIMER7，并将PC7改为AF6
	timer_disable(TIMER7);
	gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_7);  //设置为I2S2_MCK	
	i2s_enable(SPI2);	//使能I2S
	
	cprintf("  Switched to I2S 8MHz Clock, Please check...\n");
	CS_WaitKey();
	
	cprintf("Set Back to Timer Clock 8.4MHz\n");	
	i2s_disable(SPI2);	//禁用I2S
	gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_7);	//设置为Timer7输出
	timer_enable(TIMER7);
	
	cprintf("End of ADS131M02 CLK Switch\n");
	CS_WaitKey();
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

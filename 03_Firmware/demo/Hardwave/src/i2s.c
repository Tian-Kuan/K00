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
*				            PLL��   ��8MHz/5=1.6MHz; ��1.6MHz*210=336MHz����336MHz/2=168MHz��
*				            PLLI2S����8MHz/5=1.6MHz; ��1.6MHz*256=409.6MHz����409.6Mhz/5=81.92MHz����81.92MHz/10=8.192MHz
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
//#define PLL_CFG_8192		//(��Ҫ�޸���PLLʱ��)

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/

/** ---	Start of Function Code 	---------------------------------------------------------------------  **/

/************************************************************
*	�������ƣ�	I2S_PllSwitch
*	�������ܣ�	Ϊ��ӦI2Sʱ�ӣ��л�PLL����ʱ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����      PLL��Ҫ������1~4MHz�������100~500MHz��
*				Ϊ�õ�8.192MHz��Ҫ������Ϊ��
*				PLL��   ��8MHz/5=1.6MHz; ��1.6MHz*210=336MHz����336MHz/2=168MHz��
*				PLLI2S����8MHz/5=1.6MHz; ��1.6MHz*256=409.6MHz����409.6Mhz/5=81.92MHz����81.92MHz/10=8.192MHz
*				Ĭ�ϵ��������Ϊ8MHz��
*				PLL��	��8MHz/8=1MHz; ��1MHz*336=336MHz����336MHz/2=168MHz��
*				PLLI2S: ��8MHz/8=1MHz; ��1MHz*256=256MHz����256MHz/4=64MHz����64MHz/8=8MHz
************************************************************/
void I2SCLK_PllSwitch(void)
{
    //�öο��Ǹĵ�ϵͳ��ʼ����
#ifdef PLL_CFG_8192	
    rcu_pll_config(RCU_PLLSRC_HXTAL, 5, 210, 2, 7);//����Ҫ������	
    rcu_osci_stab_wait(RCU_PLL_CK);    
#endif
    
	//����I2S PLL
#ifdef PLL_CFG_8192	
    rcu_plli2s_config(256,5);       //Ҫ�����ã����8.192MHz
#else	
	rcu_plli2s_config(256,4);		//Ĭ�����ã����8MHz
#endif
	
    rcu_osci_on(RCU_PLLI2S_CK);     //����PLL
    rcu_osci_stab_wait(RCU_PLLI2S_CK);  //�ȴ�PLL����
    
    rcu_i2s_clock_config(RCU_I2SSRC_PLLI2S);    //��I2Sʱ������ΪPLLI2S���
}

/************************************************************
*	�������ƣ�	I2S_Init
*	�������ܣ�	��ʼ��I2S�Բ���MCKʱ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����      PC7 - I2S2_MCK
*               I2S2��SPI2Ϊͬһģ��
*               ������Timer7_CH1ͬʱʹ��
************************************************************/
void I2SCLK_Init(void)
{
    I2SCLK_PllSwitch();    
  
	//��Timer����ʱ���ö���Timer������
//	rcu_periph_clock_enable(RCU_GPIOC);//ʹ��GPIOCʱ��

//    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);//���ù�������PC7(I2S2_MCK)
//    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7);
// 
//    gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_7);  //����ΪI2S2_MCK
	
    rcu_periph_clock_enable(RCU_SPI2);  //ʹ��ģ��ʱ��
    
    spi_i2s_deinit(SPI2);   //ģ�鸴λ
 
#ifdef PLL_CFG_8192		
    i2s_psc_config(SPI2,I2S_AUDIOSAMPLE_32K,I2S_FRAMEFORMAT_DT16B_CH16B,I2S_MCKOUT_ENABLE); //ʹ��MCLK���, ������Ϊ10��Ƶ����������8.912MHz���ʱ��   
#else
    i2s_psc_config(SPI2, 31250u,I2S_FRAMEFORMAT_DT16B_CH16B,I2S_MCKOUT_ENABLE);	//Ĭ������8MHz���ʱ������Ϊ8��Ƶ
#endif
	
    i2s_init(SPI2, I2S_MODE_MASTERRX, I2S_STD_PHILLIPS, I2S_CKPL_LOW);  //ģ���ʼ��ΪI2S����ģʽ
   
//    i2s_enable(SPI2);   //ʹ�����
}

void I2SCLK_Test(void)
{
	cprintf("\nTest Switch ADS131M02 CLK to I2S 8MHz CLK(Old Timer CLK: 8.4MHz)\n");
	//ֹͣTIMER7������PC7��ΪAF6
	timer_disable(TIMER7);
	gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_7);  //����ΪI2S2_MCK	
	i2s_enable(SPI2);	//ʹ��I2S
	
	cprintf("  Switched to I2S 8MHz Clock, Please check...\n");
	CS_WaitKey();
	
	cprintf("Set Back to Timer Clock 8.4MHz\n");	
	i2s_disable(SPI2);	//����I2S
	gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_7);	//����ΪTimer7���
	timer_enable(TIMER7);
	
	cprintf("End of ADS131M02 CLK Switch\n");
	CS_WaitKey();
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

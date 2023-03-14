/****************************************Copyright (c)****************************************************
**                            Xi'an Ruiyida Windpower Technical Ltd. Co.
**                                      
**                                 http://www.wrdbj.com
**
** All rights reserved. Restricted to use, duplicate or disclose this code are granted through contract.
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               console.c
** Latest modified Date:    2022-5-25
** Latest Version:          1.0
** Descriptions:            Test The LED & GPIO Alarm Functions
**							
**
**--------------------------------------------------------------------------------------------------------
** Created by:              WuYong
** Created date:            2022-5-25
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
#include "led.h"
#include "console.h"

/** ---	Start of Macro define 	---------------------------------------------------------------------  **/

/** ---	Start of Para define 	---------------------------------------------------------------------  **/

/** ---	Start of Funtion define ---------------------------------------------------------------------  **/
void Test_LEDToggle(void);
void Test_LEDBlink(void);
void Test_GPIOAlarm(void);
void Test_GPIOToggle(void);

tdCS_TestList func_gpio_test[]=
{
	{Test_LEDToggle,    "Test Led toggling"             },
	{Test_LEDBlink,	    "Test Led blinking"             },
	{Test_GPIOAlarm,    "Test Gpio Alarm"               },
	{Test_GPIOToggle,	"Toggle Any GPIO Pin"			},
	{0,0}
};


/** ---	Start of Function Code 	---------------------------------------------------------------------  **/
/************************************************************
*	�������ƣ�	Led_Init
*	�������ܣ�	LED & Alarm��ʼ��
*	��ڲ�����	None
*	���ز�����	None
*	˵����      LEDָʾ     - PE2    �ߵ�ƽ����
*               ��������    - PC4    �͵�ƽ����
*               ���±���    - PC5    �͵�ƽ���� 
************************************************************/
void Led_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOE);//����GPIOE LEDʱ��
    rcu_periph_clock_enable(RCU_GPIOC);//����GPIOC Alarm��ʱ��
    
    //LED �������
    gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_2);//���ģʽ��������
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);//�������������Ƶ��2Mhz
    gpio_bit_set(GPIOE, GPIO_PIN_2);//�ߵ�ƽ���
    
    //����������������
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4);//����Ϊ����ģʽ������
    
    //���±�����������
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);//����Ϊ����ģʽ������
}
/************************************************************
*	�������ƣ�	Test_LEDToggle
*	�������ܣ�	LED״̬�л� ����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void Test_GPIOToggle(void)
{
	int iInput;
	uint8_t pinBank=1, pinNum=0;
	uint32_t gpioCTLBak, gpioPUDBak, gpioMODEBak, gpioOSPDBak;
	char ch;
	rcu_periph_enum periphRCU;
	uint32_t periphGPIO;
	
	cprintf("\nTeset Output Toggling of Any GPIO Pin\n");
	cprintf("WARNING: Can't Set Input Pin As Output, That May Cause CPU Damage!\n");
	cprintf("Do You Continue?(Y/N)(Default:N)\n");
	ch = CS_Getch();
	if(ch != 'Y' && ch != 'y')
		goto ERROR_GPIOToggle;
	cprintf("Select the bank of GPIO:\n");
	cprintf(" 1. PA Bank\n");
	cprintf(" 2. PB Bank\n");
	cprintf(" 3. PC Bank\n");
	cprintf(" 4. PD Bank\n");
	cprintf(" 5. PE Bank\n");
	cprintf(" 6. PF Bank\n");
	cprintf(" 7. PG Bank\n");
	cprintf("Your choice: ");
	if(ERROR == CS_GetIntNum(&iInput))
		goto ERROR_GPIOToggle;
	if(iInput<1 || iInput>7)
		goto ERROR_GPIOToggle;
	pinBank = iInput;	
	cprintf("  Bank %d Selected.\n", pinBank);
	
	cprintf("Input the Pin Num of GPIO(Range:0~15)(Default:0): ");
	if(ERROR == CS_GetIntNum(&iInput))
		goto ERROR_GPIOToggle;
	if(iInput<0 || iInput>15)
		goto ERROR_GPIOToggle;
	pinNum = iInput;

	cprintf("  P%c%d of GPIOs was selected\n", ('A'-1u+pinBank), pinNum);
	
	//ʹ��GPIO Bankʱ��
	periphRCU = (rcu_periph_enum) RCU_REGIDX_BIT(AHB1EN_REG_OFFSET, pinBank-1u);
	if( !(RCU_REG_VAL(periphRCU)&BIT(RCU_BIT_POS(periphRCU))))	//GPIOʱ��δʹ��
		rcu_periph_clock_enable(periphRCU);
	
	
	//����GPIOԭ����
	periphGPIO = GPIO_BASE + 0x400U*(pinBank-1u);
	gpioCTLBak = GPIO_CTL(periphGPIO);
    gpioPUDBak = GPIO_PUD(periphGPIO);
	gpioMODEBak = GPIO_OMODE(periphGPIO);
	gpioOSPDBak = GPIO_OSPD(periphGPIO);	
	
	//����ΪGPIO���
    gpio_mode_set(periphGPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BIT(pinNum));//���ģʽ��������
    gpio_output_options_set(periphGPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, BIT(pinNum));//�������������Ƶ��2Mhz

	//����100Hz�������
	cprintf("GPIO Configued OK, Begenning to Toggle, Frequency: 100Hz\n");
	cprintf("Press Any Key to Stop...\n");
	
	CS_ClearChar();	
	while(ERROR == CS_QueryChar(&ch))
	{
		gpio_bit_toggle(periphGPIO, BIT(pinNum));
		delay_1ms(5);
	}	
	
	//�ָ�GPIOԭ����
	GPIO_CTL(periphGPIO) = gpioCTLBak;
    GPIO_PUD(periphGPIO) = gpioPUDBak;
	GPIO_OMODE(periphGPIO) = gpioMODEBak;
	GPIO_OSPD(periphGPIO) = gpioOSPDBak;	
	
	ERROR_GPIOToggle:
	cprintf("End of GPIO Pin Toggle\n");
}

/************************************************************
*	�������ƣ�	Test_LEDToggle
*	�������ܣ�	LED״̬�л� ����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void Test_LEDToggle(void)
{
    char ch;
    cprintf("\nTest LED Toggle Function.\n");
    while(1)
    {
        cprintf("Toggle The Led?(Y/N)\n");
        ch = CS_Getch();
        if(ch == 'Y' || ch == 'y')
        {
            gpio_bit_toggle(GPIOE, GPIO_PIN_2);
            if(RESET == gpio_output_bit_get(GPIOE, GPIO_PIN_2))
                cprintf("  LED Darked\n");
            else
                cprintf("  LED Lighted\n");
        }
        else 
        {
            cprintf("Exit Led Toggle Test\n");
            return;
        }
    }
}

/************************************************************
*	�������ƣ�	Test_LEDBlink
*	�������ܣ�	LED��˸ ����
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void Test_LEDBlink(void)
{
    int iDelay = 1000;
    char ch;
    
    cprintf("\nTest LED Blink\n");
    cprintf("Enter the Delay Time in ms(Rang:100~10000)(Defalut:1000): ");
    if(ERROR != CS_GetIntNum(&iDelay))
    {
        if(iDelay>10000 || iDelay < 100)
            iDelay = 1000;        
    }
    cprintf("  Set Delay: %dms\n", iDelay);
    cprintf("Now Blink the LED, Press any key to Exit...\n");
    CS_ClearChar();
    while(ERROR == CS_QueryChar(&ch))
    {
        gpio_bit_toggle(GPIOE, GPIO_PIN_2);
        delay_1ms(iDelay); 
    }      
    cprintf("Exit Led Blink Test.\n");
}

/************************************************************
*	�������ƣ�	Test_GPIOAlarm
*	�������ܣ�	Alarm״̬�ı���
*	��ڲ�����	None
*	���ز�����	None
*	˵����      �͵�ƽ����
************************************************************/
void Test_GPIOAlarm(void)
{
    char ch;
    FlagStatus iTmprAlarm, iCurrAlarm, iTmprAlarmNew, iCurrAlarmNew;
    cprintf("\nTest Gpio Alarm Function\n");
    cprintf("Please Change the Temperature or Current of the Circuit and check the Alarm Change\n");
   
    iTmprAlarmNew = gpio_input_bit_get(GPIOC, GPIO_PIN_5);
    iTmprAlarm = iTmprAlarmNew;
    iCurrAlarmNew = gpio_input_bit_get(GPIOC, GPIO_PIN_4);
    iCurrAlarm = iCurrAlarmNew;
	cprintf("The Initial Temperature Alarm: %d, Current Alarm: %d\n", iTmprAlarm, iCurrAlarm);
    cprintf("Press Any key to Exit...\n");
    CS_ClearChar();
    while(ERROR == CS_QueryChar(&ch))
    {
        iTmprAlarmNew = gpio_input_bit_get(GPIOC, GPIO_PIN_5);
        if( iTmprAlarm != iTmprAlarmNew)
        {
            if(RESET == iTmprAlarmNew)
                cprintf("  !!! Over Temperature Alarmed\n");
            else
                cprintf("  Over Temperatur Alarm released\n");
            iTmprAlarm = iTmprAlarmNew;
        }
        
        iCurrAlarmNew = gpio_input_bit_get(GPIOC, GPIO_PIN_4);
        if (iCurrAlarm != iCurrAlarmNew)
        {
            if(RESET == iCurrAlarmNew)
                cprintf("  !!! Over Current Alarmed\n");
            else
                cprintf("  Over Current Alarm Released\n");
            iCurrAlarm = iCurrAlarmNew;
        }        
    } 
    cprintf("Exit Gpio Alarm Test.\n");
}

/************************************************************
*	�������ƣ�	GPIO_Test
*	�������ܣ�	����LED��Alarm����Ŀ¼
*	��ڲ�����	None
*	���ز�����	None
*	˵����      
************************************************************/
void GPIO_Test(void)
{
    MENU_Disp(func_gpio_test);
}

/** ---	End of File ---------------------------------------------------------------------------------  **/ 

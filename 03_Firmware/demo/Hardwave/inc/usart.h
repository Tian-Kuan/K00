#ifndef _USART_H_
#define _USART_H_

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"

/** ---	Start of Macro define	---------------------------------------------------------------------- **/

/** ---	Start of Funtion define	---------------------------------------------------------------------- **/
void Usart0_Init(unsigned int baud);
void Usart1_Init(unsigned int baud);
void Usart0_DMA_Init(void);

void Debug_SendStr(char *pStr);
void Debug_SendByte(char ch);
void Debug_SendNByte(char *pBuf, uint16_t size);
ErrStatus Debug_QueyChar(char *pChar);
char Debug_WaitChar(void);
void Debug_ClearChar(void);

ErrStatus Debug_DMASend(uint32_t bufAddr, uint16_t len);
FlagStatus Debug_GetDMASendingFlag(void);

void RS232_SendStr(char *pStr);
void RS232_SendByte(char ch);
void RS232_SendBuf(char *pBuf, uint16_t size);
ErrStatus RS232_QueyChar(char *pChar);
char RS232_WaitChar(void);
void RS232_ClearChar(void);

#endif
/** ---	End of File	---------------------------------------------------------------------------------- **/ 

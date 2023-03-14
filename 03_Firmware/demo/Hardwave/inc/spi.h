#ifndef _SPI_H_
#define _SPI_H_

/** ---	Start of head include	---------------------------------------------------------------------- **/
#include "gd32f4xx.h"

/** ---	Start of Macro define	---------------------------------------------------------------------- **/
#define SPI1_DMAONE_SIZE	(4)						//4*16bit(Status,ch0,ch1,crc)
#define SPI1_DMACH_LEN      (1024)
#define SPI1_DMABUF_SIZE	(SPI1_DMAONE_SIZE*SPI1_DMACH_LEN)	//4*16bit*1024: Status/CH1/CH2/CRC

#define FRAM_CSLow()       gpio_bit_reset(GPIOA, GPIO_PIN_4)
#define FRAM_CSHigh()      gpio_bit_set(GPIOA, GPIO_PIN_4)
#define FRAM_SPI            (SPI0)

#define ADS131M_CSLow()    gpio_bit_reset(GPIOB, GPIO_PIN_12)
#define ADS131M_CSHigh()   gpio_bit_set(GPIOB, GPIO_PIN_12)
#define ADS131M_SPI         (SPI1)

/** ---	Start of Funtion define	---------------------------------------------------------------------- **/
void SPI0_Init(void);	//SPI0初始化函数
void SPI1_Init(void);	//SPI1初始化函数

void SPI1_DMA_Init(void);
void SPI1_DMAReadOnce(void);
void SPI1_DMAStart(void);
void SPI1_DMAStop(void);
uint32_t SPI1_GetDMABufAddr(uint32_t mem);
//void SPI1_TIEnable(uint8_t isTI);

ErrStatus FRAM_Transmit( uint8_t *pData, uint16_t size, uint32_t timeout);
ErrStatus FRAM_Receive( uint8_t *pData, uint16_t size, uint32_t timeout);

ErrStatus FRAM_TransmitCrc( uint8_t *pData, uint16_t size, uint32_t timeout);
ErrStatus FRAM_ReceiveCrc( uint8_t *pData, uint16_t size, uint32_t timeout);
#endif
/** ---	End of File	---------------------------------------------------------------------------------- **/ 

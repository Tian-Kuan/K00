#ifndef __MCP4561_H
#define __MCP4561_H
 
/* Includes ------------------------------------------------------------------*/
#include "iic.h"  

/* Exported macro ------------------------------------------------------------*/
#define MCP4561_FULL_RANGE	(256u)

/* Exported functions ------------------------------------------------------- */
void MCP4561_Init(void);
void MCP4561_Test(void);
void MCP4561_ManualAdjust(void);

ErrStatus MCP4561_SetResistor(uint16_t val);
ErrStatus MCP4561_SaveResistor(uint16_t val);
uint16_t MCP4561_GetResistor(void);

#endif


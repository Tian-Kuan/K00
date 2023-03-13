#ifndef _PWM_H_
#define _PWM_H_

#include "gd32f4xx.h"

#define PWM_DRIVER_EN       (0x01)
#define PWM_COMPENSATE_EN   (0x02)



void PWM_Init(void);
void PWM_Test(void);
void PWM_ManuAdjust(void);

float PWM_SetCpsPhase(float fDstPhase, float fCpsPhaseOrg);
int16_t PWM_TanScale(int16_t iSin, int16_t iCos);
float PWM_Tan(int16_t iSin, int16_t iCos);
float PWM_GetCpsPhase(void);
void PWM_Enable(uint8_t state);


#endif

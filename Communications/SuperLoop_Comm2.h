#ifndef SLC2_H
#define SLC2_H

#include "board_PowerModes.h"
#include "spiffs.h"
#include "fs.h"

void SLC_init(void);
void SLC(void);


extern  e_PowerState SLC_GetPowerState(void);
extern  e_PowerState SLC_SetSleepState(bool state);
extern  bool SLC_Busy_State(void);
extern  bool SLC_SPIFFS_State(void);

void Communication_InSleep();
void Communication_OutSleep();

extern spiffs fs;



#endif

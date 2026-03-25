#ifndef BUZZER_H
#define BUZZER_H
#include "common.h"

void Buzzer_Init(void);

void Buzzer_On(void);

void Buzzer_Off(void);

void Buzzer_Beep_C4(uint16_t duration_ms);

#endif

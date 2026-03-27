#ifndef DOG_H
#define DOG_H
#include "common.h"


void IWDG_Config(void);       // 独立看门狗初始化
void WWDG_Config(void);       // 窗口看门狗初始化（可选扩展）
void System_Reset_Check(void);// 复位原因检测&用户提示


#endif

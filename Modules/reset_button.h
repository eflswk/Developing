#ifndef RESET_BUTTON_H
#define RESET_BUTTON_H
#include "common.h"

//按钮初始化
void Key_GPIO_Init(void);

//检测是否长按按钮
uint8_t Key_Check_LongPress(void);

#endif  

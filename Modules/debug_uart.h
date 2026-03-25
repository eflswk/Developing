#ifndef __DEBUG_USART1_H__
#define __DEBUG_USART1_H__

#include "common.h"

/**
 * @brief  初始化调试串口 USART1
 * @note   用于调试输出，不作为通信接口使用
 */
void DebugUART_Init(void);

/**
 * @brief  调试打印函数（串口版 printf）
 */
void printf1(const char *format, ...);

#endif

#ifndef __DELAY_H
#define __DELAY_H

/*
 * 注意：
 * 该延时函数属于“忙等待延时”
 * 在延时期间：CPU 会一直循环等待，无法执行其他任务
 * 也就是这些延时函数实现延时的过程中，会持续占用CPU
 * 因此：
 *      在 FreeRTOS 任务中，通常不建议使用该延时
 * 在 FreeRTOS 任务环境下应使用：
 *      vTaskDelay()
 * 		该函数会在延时过程中会释放CPU
 */

#include "common.h"

void Delay_Init(void);
void Delay_Us(uint16_t us);
void Delay_Ms(uint16_t ms);
void Delay_S(uint16_t s);

#endif	// !__DELAY_H

/*
 * 文件名：common.h
 * 作  用：
 *      工程统一头文件入口。
 *
 *      1. 统一包含标准库、STM32库、FreeRTOS
 *      2. 统一包含各个功能模块头文件
 *      3. 简化其他文件的 include 复杂度
 *
 * 使用方式：
 *      所有 .c 文件只需包含本头文件即可：
 *          #include "common.h"
 */


#ifndef __COMMON_H
#define __COMMON_H


/* ===================== C语言标准库 ===================== */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

/* ===================== C语言标准库 ===================== */


/* ===================== STM32 SPL ===================== */

#include "stm32f10x.h"

/* ===================== STM32 SPL ===================== */


/* ===================== FreeRTOS ===================== */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ===================== FreeRTOS ===================== */


/* ===================== 用户模块 ===================== */

#include "debug_uart.h"
#include "bluetooth.h"
#include "delay.h"
#include "common_utils.h"
#include "global.h"
#include "init.h"
#include "wifi.h"
#include "W25Q64.h"
#include "buzzer.h"
#include "leds.h"
#include "W25Q64.h"
#include "ADC.h"
#include "sensor.h"
#include "flash.h"
#include "reset_button.h"

/* ===================== 用户模块 ===================== */


/* ===================== 任务模块 ===================== */

#include "wifi_config_task.h"
#include "esp_task.h"
#include "monitor_task.h"
#include "Instruction_task.h"

/* ===================== 任务模块 ===================== */

#endif	// !__COMMON_H

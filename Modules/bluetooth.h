#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "common.h"

/*
 * 文件名：bluetooth.h
 * 作  用：
 *      HC-05 蓝牙模块头文件。
 *
 *      1. 初始化 USART3
 *      2. 接收手机端发送的 WiFi 配置字符串
 *      3. USART3 中断中逐字节将数据送入 BTMsgQueue
 */

/* 初始化 HC-05 使用的 USART3 串口 */
void BlueTooth_Init(void);

#endif /* __BLUETOOTH_H */

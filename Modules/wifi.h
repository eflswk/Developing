#ifndef __WIFI_H
#define __WIFI_H

#include "common.h"

/*
 * 文件名：wifi.h
 * 作  用：
 *      ESP-01S WiFi 模块驱动头文件。
 *
 *      1. 提供 USART2 初始化函数
 *      2. 提供基础字符串发送函数
 *
 * 说  明：
 *      1. 本模块只负责 ESP-01S 的底层串口收发
 *      2. AT 指令的组织与响应处理，不在本模块中完成
 *      3. ESP 返回数据由 USART2 中断接收后送入消息队列
 *      4. ESP 任务再从消息队列中读取并解析响应内容
 */

/**
  * @brief  初始化 ESP-01S 使用的 USART2
  * @param  无
  * @retval 无
  */
void WIFI_Init(void);

/**
  * @brief  向 ESP-01S 发送一个字符串
  * @param  Str：要发送的字符串
  * @retval 无
  */
void WIFI_SendString(char *Str);

#endif /* __WIFI_H */

#ifndef __WIFI_RECONNECT_TASK_H
#define __WIFI_RECONNECT_TASK_H

#include "common.h"

/*
 * 文件名：wifi_reconnect_task.h
 * 作  用：
 *      WiFi 配置任务头文件。
 *
 *      1. 提供 WiFi 配置任务入口函数声明
 *      2. 定义 WiFi 配置任务栈大小和优先级
 */

void WiFiReconnectTask_Entry(void *arg);

#endif

#ifndef __WIFI_CONFIG_TASK_H
#define __WIFI_CONFIG_TASK_H

#include "common.h"

/*
 * 文件名：wifi_config_task.h
 * 作  用：
 *      WiFi 配置任务头文件。
 *
 *      1. 提供 WiFi 配置任务入口函数声明
 *      2. 定义 WiFi 配置任务栈大小和优先级
 */

void WiFiConfigTask_Entry(void *arg);

#endif /* __WIFI_CONFIG_TASK_H */

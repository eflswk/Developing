#ifndef __ESP_TASK_H
#define __ESP_TASK_H

#include "common.h"

/*
 * 文件名：esp_task.h
 * 作  用：
 *      ESP 任务头文件。
 *
 *      1. 声明 ESP 任务入口函数
 *      2. 定义 ESP 任务中使用的 AT 指令宏
 *      3. 定义 ESP 响应匹配字符串宏
 *
 * 说  明：
 *      1. 本任务负责组织 AT 指令发送流程
 *      2. 本任务负责从 ESP_ATRxQueue 中读取 ESP 返回数据
 *      3. 本任务负责判断 AT 指令执行是否成功
 *      4. 本任务负责建立 WiFi 和 TCP 连接，并发送数据
 */


/*
 * 函数名：ESPTask_Entry
 * 作  用：ESP 任务入口函数
 */
void ESPTask_Entry(void *arg);

#endif /* __ESP_TASK_H */

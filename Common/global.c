/*
 * 文件名：global.c
 * 作  用：
 *      全局变量定义文件。
 *
 *      1. 为 global.h 中声明的全局资源分配内存
 *      2. 统一管理系统共享资源
 */

#include "global.h"

/* WiFi配置 */
WiFiConfig_t WiFiConfigInfo;

/* 任务句柄 */
TaskHandle_t WiFiConfigTaskHandle;
TaskHandle_t ESPTaskHandle;
// 监察任务只在需要时开启
// 可以在FreeRTOSConfig.h中配置宏configENABLE_DEBUG_STACK_MONITOR来开关
TaskHandle_t MonitorTaskHandle;
TaskHandle_t FeedDogTaskHandle;

uint8_t g_InstructionTask_RunFlag = 0;
uint8_t g_WiFi_BT_Task_RunFlag = 0;
/* 队列句柄 */
QueueHandle_t BT_MsgQueue;
QueueHandle_t ESP_ATRxQueue;



// 全局缓冲区（给封装函数用）
char s_RxBuf[ESP_RX_BUF_MAX_LEN];
char s_ATCmd[128];

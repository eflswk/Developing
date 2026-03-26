/*
 * 文件名：global.h
 * 作  用：
 *      全局变量声明管理文件。
 *
 *      1. 统一声明全局变量（extern）
 *      2. 避免多文件重复定义问题
 *      3. 提供全局资源访问入口
 *
 * 设计原则：
 *      .h 中只做全局资源声明（extern）
 *      .c 中才做相关定义
 */

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "common.h"

/* ===================== 全局类型定义 ===================== */

typedef struct {
    char SSID[20];      /* WiFi名称 */
    char PassWord[20];  /* WiFi密码 */
} WiFiConfig_t;

// 定义LED状态
typedef enum {
    LED_STATUS_IDLE,      // 未开始连接 —— 常亮
    LED_STATUS_BLINK,     // 正在连接/配网 —— 闪烁
    LED_STATUS_OFF        // 连接成功 —— 熄灭
} LED_StatusTypeDef;

/* ===================== 全局类型定义 ===================== */


/* ===================== 全局宏定义 ===================== */

/* 服务端配置 */
#define SERVER_IP      "8.136.26.114"
#define SERVER_PORT    9008

/*W25Q64存储地址*/
#define USER_ADDR  0x000000

// Flash配置标记地址（选择未被程序占用的页首地址）
#define FLASH_WIFI_FLAG_ADDR  0x0800F000

// WIFI已配置标记（自定义值，避免与擦除后全1冲突）
#define WIFI_CONFIGURED_FLAG  0x5A5A5A5A

// 按键GPIO定义（PA0）
#define RESET_KEY_GPIO_PORT   GPIOB
#define RESET_KEY_GPIO_PIN    GPIO_Pin_4

// 长按判定时间（3秒）
#define LONG_PRESS_TIME       3000

/* ===================== 全局宏定义 ===================== */


/* ===================== 全局变量仅声明 ===================== */

/* WiFi配置的全局配置区，蓝牙配网完成后需要给此结构体成员赋值 */
extern WiFiConfig_t WiFiConfigInfo;

/* 全局LED状态变量 */
extern LED_StatusTypeDef g_led_status;

/* ===================== 全局变量仅声明 ===================== */


/* ===================== FreeRTOS句柄仅声明 ===================== */

/* 任务句柄 */
extern TaskHandle_t WiFiConfigTaskHandle;    /* WiFi配置任务句柄 */
extern TaskHandle_t ESPTaskHandle;           /* ESP任务句柄 */
extern TaskHandle_t MonitorTaskHandle;		/* 监察任务句柄 */

/* 消息队列句柄 */
extern QueueHandle_t BT_MsgQueue;             /* 蓝牙接收手机端消息队列 */
extern QueueHandle_t ESP_ATRxQueue;        /* ESP接收AT反馈消息队列 */

/* ===================== FreeRTOS句柄仅声明 ===================== */


#endif // !__GLOBAL_H

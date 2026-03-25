#include "init.h"

/*
 * 文件名：init.c
 * 作  用：
 *      系统初始化实现文件。
 *
 *      1. 统一管理基础模块的初始化流程
 *      2. 消息队列的创建
 */

void System_Init(void) {
    /* 设置优先级分组4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 初始化调试串口 */
    DebugUART_Init();

    /* 初始化延时模块 */
    Delay_Init();

    /* 初始化蓝牙模块 */
    BlueTooth_Init();

	/* W25Q46模块初始化*/
	W25Q64_GPIO_Init();
	W25Q64_SPI1_Init();
	
	/* 打印之前配置的WIFI*/
	for(int i=0; i<16; i++){
    WiFiConfigInfo.SSID[i] = W25Q64_ReadByte(USER_ADDR + i);
}
	for(int i=0; i<16; i++){
    WiFiConfigInfo.PassWord[i] = W25Q64_ReadByte(USER_ADDR + 16 + i);
}
	printf1("This is last WIFI config\r\n");
    printf1("UserID: %s\r\n", WiFiConfigInfo.SSID);
    printf1("PassWord: %s\r\n", WiFiConfigInfo.PassWord);

    /* 初始化 WiFi 模块 */
    WIFI_Init();

    /* 创建蓝牙消息队列：中断中每次发送 1 个字节 */
    BT_MsgQueue = xQueueCreate(64, sizeof(uint8_t));

    if (BT_MsgQueue == NULL) {
        printf1("BTMsgQueue Create Failed.\r\n");

        while (1) {
        }
    }

    /* 创建 ESP 接收消息队列：中断中每次发送 1 个字节 */
    ESP_ATRxQueue = xQueueCreate(128, sizeof(uint8_t));

    if (ESP_ATRxQueue == NULL) {
        printf1("ESP_ATRxQueue Create Failed.\r\n");

        while (1) {
        }
    }


    printf1("System be initializing...\r\n");

    /* 延时10s，等待硬件完成初始化 */
    Delay_S(2);

    printf1("System Init OK\r\n");
}

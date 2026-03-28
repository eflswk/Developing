#ifndef __UTILS_H
#define __UTILS_H

#include "common.h"

// 解析蓝牙模块接收到的WiFi信息
void SplitWiFiInfo(char *Src, char *Name, char *Password);

void ESPTask_ClearQueue(void);

uint8_t ESPTask_WaitResponse(char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs);

uint8_t ESPTask_SendCmdAndWait(char *Cmd, char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs);

uint8_t ESP_Init(void);

uint8_t ESP_ConnectWiFi(char *ssid, char *pwd);

uint8_t ESP_ConnectTCP(void);

uint8_t ESP_SendSensorData(float light, float temp);

uint8_t ESP_NetworkConnect(void);
#endif

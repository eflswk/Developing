#include "common_utils.h"

/*
 * 函数功能：解析蓝牙发送的 WiFi 配置字符串
 * 字符串格式：!NAME=PASSWORD!
 * 例如：!MyWiFi=12345678!
 * 解析结果：
 *      Name     -> MyWiFi
 *      Password -> 12345678
 *
 * 实现思路：
 *      第一个 for 循环提取 '=' 左侧的 WiFi Name
 *      第二个 for 循环提取 '=' 与 '!' 之间的密码
 */
void SplitWiFiInfo(char *Src, char *Name, char *Password) {
    int i;
    int j;

    /* 提取 WiFi 名称，第一个字符是感叹号，从第二个字符开始切 */
    for (i = 1; Src[i] != '='; i++) {
        Name[i - 1] = Src[i];
    }

    Name[i - 1] = '\0';

    /* 提取 WiFi 密码 */
    for (j = 0; Src[i + 1 + j] != '!'; j++) {
        Password[j] = Src[i + 1 + j];
    }

    Password[j] = '\0';
}

/**
  * @brief  清空ESP接收消息队列
  */
void ESPTask_ClearQueue(void) {
    uint8_t RxData = 0;
    while (xQueueReceive(ESP_ATRxQueue, &RxData, 0) == pdPASS);
}

/**
  * @brief  等待响应
  */
uint8_t ESPTask_WaitResponse(char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs) {
    uint8_t RxData = 0;
    uint16_t Index = 0;
    TickType_t StartTick = xTaskGetTickCount();

    memset(RxBuf, 0, BufSize);
    while ((xTaskGetTickCount() - StartTick) < pdMS_TO_TICKS(TimeoutMs)) {
        if (xQueueReceive(ESP_ATRxQueue, &RxData, pdMS_TO_TICKS(20)) == pdPASS) {
            if (Index < BufSize - 1) {
                RxBuf[Index++] = (char)RxData;
                RxBuf[Index] = '\0';
            }
            if (strstr(RxBuf, Target) != NULL) {
                printf1("Match OK: %s\r\n", RxBuf);
                return 1;
            }
        }
    }
    printf1("Wait Timeout, RxBuf: %s\r\n", RxBuf);
    return 0;
}

/**
  * @brief  发送AT指令并等待
  */
uint8_t ESPTask_SendCmdAndWait(char *Cmd, char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs) {
    ESPTask_ClearQueue();
    WIFI_SendString(Cmd);
    if (ESPTask_WaitResponse(Target, RxBuf, BufSize, TimeoutMs)) {
        return 1;
    }
    return 0;
}


/**
  * @brief  ESP初始化：AT+ATE0+CWMODE
  */
uint8_t ESP_Init(void)
{
    printf1("Test AT...\r\n");
    if (!ESPTask_SendCmdAndWait(ESP_CMD_AT, ESP_RSP_OK, s_RxBuf, sizeof(s_RxBuf), 2000)) {
        printf1("AT Error\r\n");
        return 0;
    }

    printf1("Close Echo...\r\n");
    if (!ESPTask_SendCmdAndWait(ESP_CMD_ATE0, ESP_RSP_OK, s_RxBuf, sizeof(s_RxBuf), 2000)) {
        printf1("Echo Off Error\r\n");
        return 0;
    }

    printf1("Set STA Mode...\r\n");
    if (!ESPTask_SendCmdAndWait(ESP_CMD_CWMODE1, ESP_RSP_OK, s_RxBuf, sizeof(s_RxBuf), 2000)) {
        printf1("STA Mode Error\r\n");
        return 0;
    }

    return 1;
}

/**
  * @brief  连接WiFi
  */
uint8_t ESP_ConnectWiFi(char *ssid, char *pwd)
{
    printf1("Connect WiFi...\r\n");
    sprintf(s_ATCmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);

    if (ESPTask_SendCmdAndWait(s_ATCmd, ESP_RSP_OK, s_RxBuf, sizeof(s_RxBuf), 15000)) {
        printf1("Connect WiFi OK\r\n");
        g_led_status = LED_STATUS_OFF;
        vTaskDelay(pdMS_TO_TICKS(1000));
        return 1;
    } else {
        printf1("Connect WiFi Error\r\n");
        return 0;
    }
}

/**
  * @brief  连接TCP服务器
  */
uint8_t ESP_ConnectTCP(void)
{
    printf1("Connect TCP...\r\n");
    sprintf(s_ATCmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);

    if (ESPTask_SendCmdAndWait(s_ATCmd, ESP_RSP_CONNECT, s_RxBuf, sizeof(s_RxBuf), 10000)) {
        printf1("Connect TCP OK\r\n");
        return 1;
    } else {
        printf1("Connect TCP Error\r\n");
        return 0;
    }
}

/**
  * @brief  上传一条传感器数据（任务核心）
  */
uint8_t ESP_SendSensorData(float light, float temp)
{
    char BodyBuf[32] = {0};
    char HttpBuf[128] = {0};

    sprintf(BodyBuf, "light=%f,temp=%f", light, temp);
    printf1("Body: %s\r\n", BodyBuf);

    sprintf(HttpBuf, "POST /x HTTP/1.1\r\n\r\n%s", BodyBuf);

    sprintf(s_ATCmd, "AT+CIPSEND=%d\r\n", strlen(HttpBuf));
    if (!ESPTask_SendCmdAndWait(s_ATCmd, ESP_RSP_PROMPT, s_RxBuf, sizeof(s_RxBuf), 500)) {
        printf1("Begin upload failed...\r\n");
        return 0;
    }

    ESPTask_ClearQueue();
    WIFI_SendString(HttpBuf);

    if (ESPTask_WaitResponse("\r\n\r\nOK", s_RxBuf, sizeof(s_RxBuf), 5000)) {
        printf1("Server Response OK\r\n");
        return 1;
    } else {
        printf1("Server Response Error\r\n");
        return 0;
    }
}

//连网函数
uint8_t ESP_NetworkConnect(void)
{
    char SSID[20] = {0};
    char PassWord[20] = {0};

    // 从全局配置区读取WiFi信息
    strcpy(SSID, WiFiConfigInfo.SSID);
    strcpy(PassWord, WiFiConfigInfo.PassWord);

    printf1("=== Start Network Connect ===\r\n");
    printf1("SSID: %s\r\n", SSID);
    printf1("PassWord: %s\r\n", PassWord);

    // 1. 初始化ESP
    if (!ESP_Init()) {
        printf1("ESP Init Fail\r\n");
        return 0;
    }

    // 2. 连接WiFi
    if (!ESP_ConnectWiFi(SSID, PassWord)) {
        printf1("WiFi Connect Fail\r\n");
        return 0;
    }

    // 3. 连接TCP
    if (!ESP_ConnectTCP()) {
        printf1("TCP Connect Fail\r\n");
        return 0;
    }

    printf1("=== Network Connect All OK ===\r\n");
    return 1;
}

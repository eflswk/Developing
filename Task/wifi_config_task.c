#include "wifi_config_task.h"

/*
 * 文件名：wifi_config_task.c
 * 作  用：
 *      WiFi 配置任务实现文件。
 *
 *      1. 从蓝牙消息队列中逐字节接收数据
 *      2. 识别并组装形如 !MyWiFi=12345678! 的 WiFi 配置帧
 *      3. 解析出 WiFi 名称和密码
 *      4. 保存到全局 WiFiConfigInfo 中
 *      5. 配置完成后删除自身任务
 */

#define WIFI_CONFIG_FRAME_MAX_LEN    63

/**
  * @brief  WiFi 配置任务入口函数
  * @param  arg：任务参数
  * @retval 无
  */
void WiFiConfigTask_Entry(void *arg) {
    uint8_t RxData = 0;          /* 从蓝牙消息队列中取出的1个字节数据 */
    uint8_t RecvFlag = 0;        /* 帧接收标志位：0表示还未开始接收，1表示正在接收一帧 */
    uint16_t FrameIndex = 0;     /* 当前WiFi配置帧缓冲区的写入下标 */
    char FrameBuf[WIFI_CONFIG_FRAME_MAX_LEN + 1] = {0}; /* WiFi配置帧接收缓冲区 */

    printf1("WiFiConfigTask Start\r\n");
    printf1("Wait WiFi Config...\r\n");

    while (1) {
        /* 阻塞等待蓝牙发来一个字节 */
        xQueueReceive(BT_MsgQueue, &RxData, portMAX_DELAY);

        /* 接收处理起始符 '!' */
        if (RecvFlag == 0) {
            if (RxData == '!') {
                RecvFlag = 1;
                FrameIndex = 0;
                memset(FrameBuf, 0, sizeof(FrameBuf));
                FrameBuf[FrameIndex++] = (char)RxData;
                printf1("WiFi Frame Start\r\n");
            }

            continue;
        }

        /* 正在接收一帧数据，先判断是否越界 */
        if (FrameIndex >= WIFI_CONFIG_FRAME_MAX_LEN) {
            printf1("WiFi Frame Too Long\r\n");

            /* 当前帧丢弃，重新等待下一帧 */
            RecvFlag = 0;
            FrameIndex = 0;
            memset(FrameBuf, 0, sizeof(FrameBuf));
            continue;
        }

        /* 保存当前字节 */
        FrameBuf[FrameIndex++] = (char)RxData;

        /* 再次收到 '!'，说明一帧接收完成 */
        if (RxData == '!') {
            FrameBuf[FrameIndex] = '\0';
            printf1("WiFi Frame Done: %s\r\n", FrameBuf);
            /* 先清空全局WiFi配置区 */
            memset(WiFiConfigInfo.SSID, 0, sizeof(WiFiConfigInfo.SSID));
            memset(WiFiConfigInfo.PassWord, 0, sizeof(WiFiConfigInfo.PassWord));
            /* 解析字符串，并把WiFi信息存入全局变量结构体 */
            SplitWiFiInfo(FrameBuf, WiFiConfigInfo.SSID, WiFiConfigInfo.PassWord);

            printf1("WiFi Config OK\r\n");
            printf1("SSID: %s\r\n", WiFiConfigInfo.SSID);
            printf1("PassWord: %s\r\n", WiFiConfigInfo.PassWord);
			/* 保存此次的WIFI配置*/
			W25Q64_SectorErase(USER_ADDR); // 先擦除

			for(int i=0; i<16; i++){
				W25Q64_WriteByte(USER_ADDR + i, WiFiConfigInfo.SSID[i]);
			}
			for(int i=0; i<16; i++){
				W25Q64_WriteByte(USER_ADDR + 16 + i, WiFiConfigInfo.PassWord[i]);
			}
            /* 创建ESP联网、TCP通信任务，优先级为1 */
            BaseType_t   Ret = xTaskCreate(ESPTask_Entry,
                                           "ESPTask",
			// ESP任务栈的大小不能使用初始最小大小，不够用！
                                           configMINIMAL_STACK_SIZE * 2,
                                           NULL,
                                           tskIDLE_PRIORITY + 1,
                                           &ESPTaskHandle);

            if (Ret == pdPASS) {
                printf1("ESPTask Create OK\r\n");
            } else {
                printf1("ESPTask Create Error\r\n");

                while (1);
            }

            printf1("WiFiConfigTask Delete\r\n");
            vTaskDelete(NULL);
        }
    }
}

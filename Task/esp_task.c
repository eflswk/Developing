#include "esp_task.h"

/*
 * 文件名：esp_task.c
 * 作  用：
 *      ESP任务实现文件。
 *
 *      1. 从全局配置区读取WiFi信息
 *      2. 发送AT指令并等待响应
 *      3. 连接WiFi
 *      4. 建立TCP连接
 *      5. 周期性发送HTTP请求
 *
 * 说明：
 *      1. 本任务处理的是 ESP 返回的串口文本流，例如：
 *         OK、WIFI GOT IP、CONNECT、SEND OK 等。
 *      2. 因此这里采用“边接收、边拼接、边查找目标字符串”的方式。
 *         也就是：从消息队列中逐字节取出ESP返回的数据，
 *         依次保存到本地缓冲区中，然后通过 strstr() 判断
 *         目标字符串是否已经出现。
 *      3. 这和前面的 WiFiConfigTask 不同。
 *         WiFiConfigTask 处理的是 !SSID=PASSWORD! 这种完整配置帧，
 *         有明显的起始符和结束符，因此更适合先组完整帧，再统一解析。
 *      4. 一旦关键步骤失败，先打印调试信息，然后直接死循环停住，
 *         方便调试定位问题。
 */
 
 /* ESP 接收缓冲区最大长度 */
#define ESP_RX_BUF_MAX_LEN               256

/* ==================== ESP 常用 AT 指令 ==================== */

/* 测试 AT */
#define ESP_CMD_AT                       "AT\r\n"

/* 关闭回显 */
#define ESP_CMD_ATE0                     "ATE0\r\n"

/* 设置为 STA 模式 */
#define ESP_CMD_CWMODE1                  "AT+CWMODE=1\r\n"

/* ==================== ESP 常用响应字符串 ==================== */

/* 普通 AT 指令执行成功 */
#define ESP_RSP_OK                       "\r\nOK\r\n"

/* 连接热点成功 */
#define ESP_RSP_WIFI_GOT_IP              "WIFI GOT IP"

/* 建立 TCP 连接成功 */
#define ESP_RSP_CONNECT                  "CONNECT"

/* CIPSEND 后的发送提示符 */
#define ESP_RSP_PROMPT                   ">"

/* 数据发送成功 */
#define ESP_RSP_SEND_OK                  "SEND OK"

/* 服务端响应成功 */
#define ESP_RSP_SERVER_OK                "OK"

/**
  * @brief  清空ESP接收消息队列
  * @note   发送新指令前先清掉旧数据
  * 		由于我们对ESP反馈的处理是查找关键字符串
  *    		所以很经常出现一种情况，消息队列的数据读到一半已经判定成功
  *			那么这些残留数据就会仍然存在消息队列中
  *			这些数据不清理显然影响下一次AT指令的发送与接收反馈
  */
static void ESPTask_ClearQueue(void) {
    uint8_t RxData = 0; /* 用于临时接收并丢弃队列中的旧字节 */

    // pdPASS意味着消息队列还有数据，将它们全部读出，直到清空为止
    while (xQueueReceive(ESP_ATRxQueue, &RxData, 0) == pdPASS);
}


/**
  * @brief  发送AT指令后，等待指定响应字符串，从ESP_ATRxQueue中逐字节接收并进行字符串匹配
  * @param  Target：目标响应字符串
  * @param  RxBuf：接收缓冲区
  * @param  BufSize：接收缓冲区大小
  * @param  TimeoutMs：等待超时时间，单位ms
  * @retval 1：收到目标字符串
  * @retval 0：超时未收到
  */
static uint8_t ESPTask_WaitResponse(char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs) {
    uint8_t RxData = 0;                     /* 每次从消息队列中取出的1个字节 */
    uint16_t Index = 0;                     /* 接收缓冲区当前写入下标 */
    TickType_t StartTick = xTaskGetTickCount(); /* 记录开始等待时的Tick值 */

    /* 每次开始等待前，先把本地接收缓冲区清空 */
    memset(RxBuf, 0, BufSize);
    /* 在超时时间内循环等待目标响应 */
    while ((xTaskGetTickCount() - StartTick) < pdMS_TO_TICKS(TimeoutMs)) {
        /* 每次最多等待100ms，看队列中是否收到新的ESP返回字节 */
        if (xQueueReceive(ESP_ATRxQueue, &RxData, pdMS_TO_TICKS(20)) == pdPASS) {		
            /* 只要缓冲区还没满，就把收到的字节保存进去 */
            if (Index < BufSize - 1) {
                RxBuf[Index++] = (char)RxData;
                RxBuf[Index] = '\0';
            }

            /* 每收到一点数据，就检查一次目标字符串是否已经出现 */
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
  * @brief  实现发送AT指令并等待响应的整个过程
  * @param  Cmd：要发送的AT指令（注意要以\r\n结尾）
  * @param  Target：期望收到的目标响应字符串
  * @param  RxBuf：接收缓冲区
  * @param  BufSize：接收缓冲区大小
  * @param  TimeoutMs：等待超时时间，单位ms
  * @retval 1：成功
  * @retval 0：失败
  */
static uint8_t ESPTask_SendCmdAndWait(char *Cmd, char *Target, char *RxBuf, uint16_t BufSize, uint32_t TimeoutMs) {
    /* 发送新指令前，先把旧的接收数据清掉 */
    ESPTask_ClearQueue();
    /* 发送AT指令字符串 */
    WIFI_SendString(Cmd);
    /* 等待指定响应 */
    if (ESPTask_WaitResponse(Target, RxBuf, BufSize, TimeoutMs)) {
        return 1;
    }

    return 0;
}

/**
  * @brief  ESP任务入口函数
  * @param  arg：任务参数
  * @note   完成联网并周期性发送HTTP数据
  */
void ESPTask_Entry(void *arg) {
    /* 保存从全局配置区中取出的WiFi名称和密码 */
    char SSID[20] = {0};
    char PassWord[20] = {0};

    /* ESP响应接收缓冲区，用来保存AT指令返回的文本内容 */
    char RxBuf[ESP_RX_BUF_MAX_LEN] = {0};
    /* AT指令拼接缓冲区，用来构造带参数的AT命令 */
    char ATCmd[128] = {0};

    /* 第一步：从全局配置区读取WiFi信息 */
    strcpy(SSID, WiFiConfigInfo.SSID);
    strcpy(PassWord, WiFiConfigInfo.PassWord);

    printf1("ESPTask Start\r\n");
    printf1("Get WiFi Config\r\n");
    printf1("SSID: %s\r\n", SSID);
    printf1("PassWord: %s\r\n", PassWord);

    /* 第二步：测试AT */
    printf1("Test AT...\r\n");
    if (ESPTask_SendCmdAndWait("AT\r\n", ESP_RSP_OK, RxBuf, sizeof(RxBuf), 2000)) {
        printf1("AT OK\r\n");
    } else {
        printf1("AT Error\r\n");
        while (1);
    }

    /* 第三步：关闭回显 */
    printf1("Close Echo...\r\n");

    if (ESPTask_SendCmdAndWait(ESP_CMD_ATE0, ESP_RSP_OK, RxBuf, sizeof(RxBuf), 2000)) {
        printf1("Echo Off OK\r\n");
    } else {
        printf1("Echo Off Error\r\n");

        while (1);
    }

    /* 第四步：设置STA模式 */
    printf1("Set STA Mode...\r\n");

    if (ESPTask_SendCmdAndWait(ESP_CMD_CWMODE1, ESP_RSP_OK, RxBuf, sizeof(RxBuf), 2000)) {
        printf1("STA Mode OK\r\n");
    } else {
        printf1("STA Mode Error\r\n");

        while (1);
    }

    /* 第五步：连接WiFi */
    printf1("Connect WiFi...\r\n");
    sprintf(ATCmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PassWord);

    if (ESPTask_SendCmdAndWait(ATCmd, ESP_RSP_OK, RxBuf, sizeof(RxBuf), 15000)) {
        printf1("Connect WiFi OK\r\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
        printf1("Connect WiFi Error\r\n");

        while (1);
    }

    /* 第六步：建立TCP连接 */
    printf1("Connect TCP...\r\n");
    sprintf(ATCmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);

    if (ESPTask_SendCmdAndWait(ATCmd, ESP_RSP_CONNECT, RxBuf, sizeof(RxBuf), 10000)) {
        printf1("Connect TCP OK\r\n");
    } else {
        printf1("Connect TCP Error\r\n");
        while (1);
    }

    /* HTTP请求体缓冲区，例如：light=11,temp=21 */
    char BodyBuf[32] = {0};
    /* HTTP请求报文缓冲区，例如：POST /x HTTP/1.1\r\n\r\nlight=11,temp=21 */
    char HttpBuf[128] = {0};

    int Light = 10;
    int Temp = 20;

    TickType_t LastUploadTime = xTaskGetTickCount();

    /* 第七步开始：周期性发送HTTP请求 */
    while (1) {
        /* 构造假传感器数据，每次略微变化 */
        Light ++;
        Temp ++;

        printf1("Build Sensor Data...\r\n");
        sprintf(BodyBuf, "light=%d,temp=%d", Light, Temp);
        printf1("Body: %s\r\n", BodyBuf);

        /* 构造HTTP请求报文 */
        printf1("Build HTTP Request...\r\n");
        sprintf(HttpBuf, "POST /x HTTP/1.1\r\n\r\n%s", BodyBuf);

        /* 先发送CIPSEND */
        printf1("Send CIPSEND...\r\n");
        sprintf(ATCmd, "AT+CIPSEND=%d\r\n", strlen(HttpBuf));

        if (ESPTask_SendCmdAndWait(ATCmd, ESP_RSP_PROMPT, RxBuf, sizeof(RxBuf), 500)) {
            printf1("Begin upload data...\r\n");
        } else {
            printf1("Begin upload failed...\r\n");

            while (1);
        }

        /* 发送真正的HTTP请求 */
        printf1("Send HTTP Request...\r\n");
        ESPTask_ClearQueue();
        WIFI_SendString(HttpBuf);

        /* 等待服务端响应 */
        printf1("Wait Server Response...\r\n");

        if (ESPTask_WaitResponse("\r\n\r\nOK", RxBuf, sizeof(RxBuf), 5000)) {
            printf1("Server Response OK\r\n");
        } else {
            printf1("Server Response Error\r\n");
            while (1);
        }

        /* 周期性任务，每10秒执行一次 */
        vTaskDelayUntil(&LastUploadTime, 10000);
    }
}

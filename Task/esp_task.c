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
 


/**
  * @brief  ESP任务入口函数
  */
void ESPTask_Entry(void *arg) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xTaskPeriod = pdMS_TO_TICKS(40);
    ESP_NetworkConnect();
    // if(g_WiFi_Connect_State == 0&&OutlineTaskHandle == NULL){
    //     xTaskCreate(Outline_Entry, "OutlineTask", 
    //             configMINIMAL_STACK_SIZE * 3, 
    //             NULL, 
    //             tskIDLE_PRIORITY + 1, 
    //             &OutlineTaskHandle);
    // }

    float Light = 0;
    float Temp = 0;
    TickType_t LastUploadTime = xTaskGetTickCount();

    // ====================== 循环只做：采集 + 上传 ======================
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);

        // 采集数据
        Light = Get_Light_Intensity();
        Temp = Get_Temperature();

        // 蜂鸣器
        if(Temp > 30){
            Buzzer_On();
        }

        // 上传数据
        ESP_SendSensorData(Light, Temp);

        g_WiFi_BT_Task_RunFlag = 1;

        // 10秒上传一次
        vTaskDelayUntil(&LastUploadTime, pdMS_TO_TICKS(10000));
    }
}

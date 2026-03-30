#include "wifi_reconnect_task.h"

/**
 * @brief  WiFi重连任务：周期性尝试重新连接WiFi+TCP，成功后删除自身
 * @param  arg 任务参数（未使用）
 */
void WiFiReconnectTask_Entry(void *arg)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xReconnectPeriod = pdMS_TO_TICKS(10000);  // 10秒重连一次

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xReconnectPeriod);

        printf1("=== WiFi Reconnect Try ===\r\n");
        // 尝试重新联网（复用原有逻辑）
        if (ESP_NetworkConnect() == 1)
        {
            // 重连成功：更新状态 + 删除自身任务 + 重置标志
            g_WiFi_Connect_State = 1;
            g_WiFi_Reconnect_Task_Created = 0;
            printf1("WiFi Reconnect Success! Delete Reconnect Task\r\n");
            
            // 恢复ESP任务的正常数据上传（如果需要）
            if (ESPTaskHandle == NULL)
            {
                xTaskCreate(ESPTask_Entry, "ESPTask",
                            configMINIMAL_STACK_SIZE * 2,
                            NULL,
                            tskIDLE_PRIORITY + 1,
                            &ESPTaskHandle);
            }

            vTaskDelete(WiFiReconnectTaskHandle);  // 删除自身
            WiFiReconnectTaskHandle = NULL;        // 清空句柄
            break;
        }
        else
        {
            g_WiFi_Connect_State = 0;
            printf1("WiFi Reconnect Fail! Retry after 5s\r\n");
        }

        // 更新喂狗标志（避免看门狗判定任务卡死）
        g_WiFi_BT_Task_RunFlag = 1;
    }
}

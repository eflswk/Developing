#include "common.h"

// 全局状态，默认未开始 —— 常亮
LED_StatusTypeDef g_led_status = LED_STATUS_IDLE;

// LED 独立任务
void Instruction_Task(void *arg)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xTaskPeriod = pdMS_TO_TICKS(200);
    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);
        switch(g_led_status)
        {
            case LED_STATUS_IDLE:
                LEDS_On();  // 常亮
                vTaskDelay(100);
                break;

            case LED_STATUS_BLINK:
                LEDS_Blinking(); // 闪烁
                vTaskDelay(300);
                break;

            case LED_STATUS_OFF:
                LEDS_Off(); // 熄灭
                vTaskDelay(100);
                break;
        }

        // 新增WiFi状态检测 
        // 场景1：WiFi未连接，且重连任务未创建 → 创建重连任务
        if (g_WiFi_Connect_State == 0 && g_WiFi_Reconnect_Task_Created == 0)
        {
            printf1("WiFi Disconnected! Create Reconnect Task\r\n");
            BaseType_t ret = xTaskCreate(WiFiReconnectTask_Entry,
                                         "WiFiReconnectTask",
                                         configMINIMAL_STACK_SIZE * 2,  // 栈大小和ESPTask一致
                                         NULL,
                                         tskIDLE_PRIORITY + 1,
                                         &WiFiReconnectTaskHandle);
            if (ret == pdPASS)
            {
                g_WiFi_Reconnect_Task_Created = 1;  // 标记任务已创建
                g_led_status = LED_STATUS_BLINK;     // LED闪烁提示重连中
            }
            else
            {
                printf1("Create WiFiReconnectTask Fail!\r\n");
            }
        }
        // 场景2：WiFi已连接，且重连任务已创建 → 强制删除重连任务（兜底逻辑）
        else if (g_WiFi_Connect_State == 1 && g_WiFi_Reconnect_Task_Created == 1)
        {
            if (WiFiReconnectTaskHandle != NULL)
            {
                vTaskDelete(WiFiReconnectTaskHandle);
                WiFiReconnectTaskHandle = NULL;
                g_WiFi_Reconnect_Task_Created = 0;
                g_led_status = LED_STATUS_OFF;  // LED熄灭表示连接成功
                printf1("WiFi Connected! Reconnect Task Deleted\r\n");
            }
        }

        
         // 3. 检测是否长按复位键

    // ==================== 长按复位 ====================
        if(Key_Check_LongPress() == 1)
        {
            printf1("press 3sec then reset\r\n");
            Flash_Clear_WIFI_Flag();   // 清除缓冲区
            printf1("reseting wifi moudle...\r\n");
            vTaskDelay(1000);
            NVIC_SystemReset();
        }
    //更新任务运行标志（告诉喂狗任务“我还活着”）
    g_InstructionTask_RunFlag = 1;
    }
}

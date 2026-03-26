#include "common.h"

// 全局状态，默认未开始 —— 常亮
LED_StatusTypeDef g_led_status = LED_STATUS_IDLE;


// LED 独立任务
void Instruction_Task(void *arg)
{
    while(1)
    {
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

         // 2. 检测是否长按复位键

    // ==================== 长按复位 ====================
        if(Key_Check_LongPress() == 1)
        {
            printf1("press 3sec then reset\r\n");
            Flash_Clear_WIFI_Flag();   // 清除缓冲区
            printf1("reseting wifi moudle...\r\n");
            vTaskDelay(1000);
            NVIC_SystemReset();
        }

    }
}

#include "Instruction_Task.h"

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
    }
}

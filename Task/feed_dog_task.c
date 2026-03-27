#include "feed_dog_task.h"

/**
 * @brief  看门狗喂狗任务（FreeRTOS任务）
 * @param  pvParameters: 任务参数（未使用）
 * @retval 无
 * @note   1. 周期300ms喂IWDG（需在410ms超时前）
 *         2. 检查核心任务运行标志，异常则停止喂狗（触发复位）
 *         3. WWDG由中断自动喂狗，此处仅监控任务标志
 */
void FeedDog_Task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFeedPeriod = pdMS_TO_TICKS(IWDG_FEED_PERIOD_MS);

    while(1)
    {
        // 等待喂狗周期（精准延时）
        vTaskDelayUntil(&xLastWakeTime, xFeedPeriod);

        // ========== 第一步：检查核心任务是否正常运行 ==========
        // 1. 检查LED指令任务标志（需在Instruction_Task中周期性置1）
        if(g_InstructionTask_RunFlag == 0)
        {
            printf1("[Warning] Instruction_Task stuck! Stop feed IWDG\r\n");
            // 任务卡死，停止喂狗 → IWDG超时复位
            for(;;);
        }

        // 2. 检查WiFi/蓝牙任务标志（需在对应任务中周期性置1）
        if(g_WiFi_BT_Task_RunFlag == 0 && (ESPTaskHandle || WiFiConfigTaskHandle))
        {
            printf1("[Warning] WiFi/BT Task stuck! Stop feed IWDG\r\n");
            // 任务卡死，停止喂狗 → IWDG超时复位
            for(;;);
        }

        // ========== 第二步：核心任务正常 → 喂IWDG ==========
        IWDG_ReloadCounter();  // 喂独立看门狗（重置计数器）
        printf1("[Info] Feed IWDG success, system normal\r\n");

        // ========== 第三步：重置任务运行标志（等待下一次任务更新） ==========
        g_InstructionTask_RunFlag = 0;
        if(!(Flash_Read_WIFI_Flag() == 1 && ESPTaskHandle) && !WiFiConfigTaskHandle)
        {
            g_WiFi_BT_Task_RunFlag = 0;  // 无WiFi/蓝牙任务时重置
        }
    }
}

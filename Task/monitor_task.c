#include "monitor_task.h"

/*
 * 函数名：MonitorTask_PrintStackHighWaterMark
 * 作  用：打印指定任务的栈最高水位
 *
 * 说  明：
 *      1. uxTaskGetStackHighWaterMark 返回值单位是 word
 *      2. 1 word = 4 字节
 *      3. 该值表示该任务历史上最少还剩多少栈空间
 */
#if ( configENABLE_DEBUG_STACK_MONITOR == 1 )

#define MonitorTask_PERIOD_MS      5000

/*
 * 函数名：vApplicationStackOverflowHook
 * 作  用：任务栈溢出回调函数
 *
 * 说  明：
 *      1. 当 FreeRTOS 检测到任务栈溢出时，会自动调用该函数
 *      2. 该函数中打印发生栈溢出的任务名
 *      3. 进入死循环，便于调试定位问题
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf1("Stack Overflow!!! Task = %s\r\n", pcTaskName);
    while (1) {
    }
}

static void MonitorTask_PrintStackHighWaterMark(const char *TaskName,
        TaskHandle_t TaskHandle) {

    if (TaskHandle == NULL) {
        return;
    }

    UBaseType_t HighWaterMark = uxTaskGetStackHighWaterMark(TaskHandle);

    printf1("%s HighWaterMark = %u word, %u byte\r\n",
            TaskName,
            (unsigned int)HighWaterMark,
            (unsigned int)(HighWaterMark * 4));
}

/*
 * 函数名：MonitorTask_Create
 * 作  用：创建监察任务
 */
void MonitorTask_Create(void) {
    BaseType_t Ret = xTaskCreate(MonitorTask_Entry,
                      "MonitorTask",
                      configMINIMAL_STACK_SIZE,
                      NULL,
                      tskIDLE_PRIORITY + 2,
                      &MonitorTaskHandle);

    if (Ret == pdPASS) {
        printf1("MonitorTask Create OK\r\n");
    } else {
        printf1("MonitorTask Create Error\r\n");
    }
}

/*
 * 函数名：MonitorTask_Entry
 * 作  用：监察任务入口函数
 *
 * 说  明：
 *      1. 周期性打印重点任务的栈最高水位
 *      2. 当前默认监控 WiFiConfigTask、ESPTask、MonitorTask 自身
 *      3. 后续如果还有其他任务，也可以继续添加
 */
void MonitorTask_Entry(void *argument) {
    TickType_t LastPrintTime = xTaskGetTickCount();

    while (1) {
        MonitorTask_PrintStackHighWaterMark("WiFiConfigTask", WiFiConfigTaskHandle);
        MonitorTask_PrintStackHighWaterMark("ESPTask", ESPTaskHandle);
        MonitorTask_PrintStackHighWaterMark("MonitorTask", MonitorTaskHandle);

        printf1("\r\n");
        vTaskDelayUntil(&LastPrintTime, pdMS_TO_TICKS(MonitorTask_PERIOD_MS));
    }
}

#endif

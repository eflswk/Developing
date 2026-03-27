/***************************************************************************************************
* USART1 调试串口（USB转串口）
*
* 接线说明：
*   PA9   (TX)  --->  RXD
*   PA10  (RX)  <---  TXD
*   GND         --->  GND
*
* 注意事项：
*   1. TX 与 RX 必须交叉连接
*
***************************************************************************************************/

/***************************************************************************************************
* USART3 蓝牙模块（HC-05）
*
* 接线说明：
*   PB10  (TX)  --->  RXD
*   PB11  (RX)  <---  TXD
*   5V          --->  VCC
*   GND         --->  GND
*
* 注意事项：
*   1. TX 与 RX 必须交叉连接
*   2. 建议使用独立 5V 电源供电（避免供电不稳）
*
***************************************************************************************************/

/***************************************************************************************************
* USART2 WiFi模块（ESP-01S）
*
* 接线说明：
*   PA2   (TX)  --->  RX
*   PA3   (RX)  <---  TX
*   3.3V        --->  VCC
*   GND         --->  GND
*   3.3V        --->  EN
*   3.3V        --->  RST
*
* 注意事项：
*   1. 必须使用 3.3V 供电（严禁 5V）
*   2. 建议使用独立 3.3V 电源（WiFi瞬时电流较大）
*   3. EN、RST 必须保持高电平
*
***************************************************************************************************/

#include "common.h"

int main(void)
{
    // 系统初始化
    System_Init();
    // 创建 LED 任务
    xTaskCreate(Instruction_Task, "Instruction_Task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 1,
                NULL);

    // ====================== 核心逻辑 ======================
    if(Flash_Read_WIFI_Flag() == 1)  //读flash判断是否连接成功
    {
        // 上次已经连接过wifi → 直接启动 ESP 任务
        printf1("Wifi used to be connected, reconnecting\r\n");

        xTaskCreate(ESPTask_Entry, "ESPTask",
                    configMINIMAL_STACK_SIZE * 2,
                    NULL,
                    tskIDLE_PRIORITY + 1,
                    &ESPTaskHandle);
    }
    else
    {
        // 未配网 → 创建蓝牙配网任务
        printf1("Not connected, creat Wificonfig task\r\n");

        BaseType_t xRet = xTaskCreate(WiFiConfigTask_Entry,
                                      "WiFiCfgTask",
                                      configMINIMAL_STACK_SIZE,
                                      NULL,
                                      tskIDLE_PRIORITY + 2,
                                      &WiFiConfigTaskHandle);
        if(xRet == pdPASS) {
        printf1("WiFiConfigTask Create OK\r\n");
    } else {
        printf1("WiFiConfigTask Create Error\r\n");
    }
}
	#if ( configENABLE_DEBUG_STACK_MONITOR == 1 )
	MonitorTask_Create();
	#endif 

    /* 启动任务调度器 */
    vTaskStartScheduler();

    /* 正常情况下不会运行到这里 */
    while (1) {
    }
}

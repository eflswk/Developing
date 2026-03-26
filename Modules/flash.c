#include "flash.h"

/**
 * @brief  读取Flash中的WIFI配置标记
 * @retval 0：未配置；1：已配置
 */
uint8_t Flash_Read_WIFI_Flag(void)
{
    uint32_t flag = *(__IO uint32_t *)(FLASH_WIFI_FLAG_ADDR);
    if(flag == WIFI_CONFIGURED_FLAG)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  写入WIFI已配置标记到Flash
 */
void Flash_Write_WIFI_Flag(void)
{
    FLASH_Unlock();                          // 解锁Flash
    FLASH_ErasePage(FLASH_WIFI_FLAG_ADDR);   // 擦除目标页
    // 写入32位标记（STM32F1需拆分为两个半字，标准库函数自动处理）
    FLASH_ProgramWord(FLASH_WIFI_FLAG_ADDR, WIFI_CONFIGURED_FLAG);
    FLASH_Lock();                            // 锁定Flash
}

/**
 * @brief  清除Flash中的WIFI配置标记
 */
void Flash_Clear_WIFI_Flag(void)
{
    FLASH_Unlock();                          // 解锁Flash
    FLASH_ErasePage(FLASH_WIFI_FLAG_ADDR);   // 擦除目标页（擦除后为0xFFFFFFFF）
    FLASH_Lock();                            // 锁定Flash
}

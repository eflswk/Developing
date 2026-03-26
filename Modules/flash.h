#ifndef FLASH_H
#define FLASH_H
#include "common.h"
/**
 * @brief  读取Flash中的WIFI配置标记
 * @retval 0：未配置；1：已配置
 */
uint8_t Flash_Read_WIFI_Flag(void);

/**
 * @brief  写入WIFI已配置标记到Flash
 */
void Flash_Write_WIFI_Flag(void);


/**
 * @brief  清除Flash中的WIFI配置标记
 */
void Flash_Clear_WIFI_Flag(void);
#endif  

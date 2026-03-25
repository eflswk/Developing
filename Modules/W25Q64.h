#ifndef __W25Q64_H
#define __W25Q64_H

#include "common.h"

typedef enum {
    PC13_STATE_OFF = 0x00,   // 指示灯熄灭
    PC13_STATE_ON  = 0x01    // 指示灯点亮
} PC13State;

/**
* @brief  初始化 W25Q64 模块（SPI1 GPIO部分）
*/
void W25Q64_GPIO_Init(void);

/**
* @brief  初始化 W25Q64 模块（SPI1外设初始化）
*/
void W25Q64_SPI1_Init(void);

/**
* @brief  读取 W25Q64 的 JEDEC ID
*/
void W25Q64_ReadJEDECID(uint8_t *ManufacturerID,
                        uint8_t *MemoryType,
                        uint8_t *Capacity);

/**
* @brief  擦除指定扇区（4KB）
*/
void W25Q64_SectorErase(uint32_t Address);

/**
* @brief  页编程（写多个字节）
*/
void W25Q64_WriteBytes(uint32_t Address,
                       uint8_t *Bytes,
                       uint16_t Length);

/**
* @brief  页编程（写1个字节）
*/
void W25Q64_WriteByte(uint32_t Address, uint8_t Byte);

/**
* @brief  从 W25Q64 中读取 1 个字节数据
*/
uint8_t W25Q64_ReadByte(uint32_t Address);


/**
 * @brief  在 W25Q64 中存储 PC13 的状态（断电保持）
 * @param  State PC13 的逻辑状态（枚举值）
 * @retval 无
 * @note   写入前必须擦除；擦除以扇区为单位（4KB），因此需要先对齐扇区首地址
 */
void W25Q64_SavePC13State(PC13State State);

PC13State W25Q64_RestorePC13State(void);

#endif

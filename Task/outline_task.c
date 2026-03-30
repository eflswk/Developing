#include "outline_task.h"
#include "common_utils.h"
/**
 * @brief  计算当前地址所在扇区的起始地址
 */
static uint32_t W25Q64_GetSectorStartAddr(uint32_t addr)
{
    return addr & ~(W25Q64_SECTOR_SIZE - 1);
}

/**
 * @brief  格式化传感器数据
 */
static uint16_t FormatSensorData(float light, float temp, char *buf)
{
    return sprintf(buf, "light=%.1f,temp=%.1f;", light, temp);
}

/**
 * @brief  读取Flash中所有离线数据（修复版：正确识别逐条存储的格式）
 */
static uint16_t Outline_ReadAllData(char *buf, uint16_t max_len)
{
    uint32_t addr = W25Q64_DATA_START_ADDR;
    uint16_t len = 0;
    uint8_t read_byte = 0;

    if(g_W25Q64_Current_Write_Addr <= W25Q64_DATA_START_ADDR)
        return 0;

    // 循环读取每个字节，直到写地址/缓冲区满，保留原始的分号分隔格式
    while(addr < g_W25Q64_Current_Write_Addr && len < max_len - 1)
    {
        read_byte = W25Q64_ReadByte(addr++);
        // 过滤空字节（Flash擦除后是0xFF，避免干扰）
        if (read_byte != 0xFF && read_byte != 0x00)
        {
            buf[len++] = read_byte;
        }
    }
    // 确保字符串结束符，且最后一个字符如果是多余分号则去掉（可选，服务端兼容的话可省略）
    if (len > 0 && buf[len-1] == ';')
    {
        buf[len-1] = 0;
        len--;
    }
    else
    {
        buf[len] = 0;
    }
    
    printf1("Read all offline data: %s (len=%d)\r\n", buf, len); // 新增日志，排查数据内容
    return len;
}

/**
 * @brief  清空离线数据区
 */
static void Outline_ClearFlashData(void)
{
    uint32_t start = W25Q64_GetSectorStartAddr(W25Q64_DATA_START_ADDR);
    uint32_t end   = W25Q64_GetSectorStartAddr(g_W25Q64_Current_Write_Addr);

    for(uint32_t s=start; s<=end; s+=W25Q64_SECTOR_SIZE)
    {
        W25Q64_SectorErase(s);
    }

    g_W25Q64_Current_Write_Addr = W25Q64_DATA_START_ADDR;
    g_W25Q64_Current_Sector_Erased = 0;

    printf1("Offline flash cleared\r\n");
}

/**
 * @brief  上传离线数据（/y 多组格式）
 */
static uint8_t Outline_UploadToServer(void)
{
    char data_buf[1024] = {0};
    uint16_t len = Outline_ReadAllData(data_buf, sizeof(data_buf));

    if(len == 0)
    {
        printf1("No offline data\r\n");
        return 1; // 无数据也算“逻辑成功”，避免误判
    }

    printf1("Upload offline data to /y: %s\r\n", data_buf); // 新增：打印要上传的完整数据
    // 直接调用你项目兼容的多组上传函数
    if(ESP_SendMultiSensorData(data_buf))
    {
        printf1("Upload offline data success\r\n");
        return 1;
    }
    else
    {
        printf1("Upload offline data failed\r\n");
        return 0;
    }
}

/**
 * @brief  写入Flash（你原来的代码，完全不动）
 */
static void Outline_SaveDataToW25Q64(char *data, uint16_t len)
{
    uint32_t current_sector = W25Q64_GetSectorStartAddr(g_W25Q64_Current_Write_Addr);
    uint32_t next_write_addr = g_W25Q64_Current_Write_Addr + len;

    if ((!g_W25Q64_Current_Sector_Erased) ||
        (W25Q64_GetSectorStartAddr(next_write_addr) != current_sector))
    {
        W25Q64_SectorErase(current_sector);
        g_W25Q64_Current_Sector_Erased = 1;
    }

    uint16_t remain_bytes_in_page = W25Q64_PAGE_SIZE - (g_W25Q64_Current_Write_Addr % W25Q64_PAGE_SIZE);
    uint16_t write_len = len;
    uint8_t *data_ptr = (uint8_t *)data;

    while (write_len > 0)
    {
        uint16_t step_len = (write_len > remain_bytes_in_page) ? remain_bytes_in_page : write_len;
        W25Q64_WriteBytes(g_W25Q64_Current_Write_Addr, data_ptr, step_len);

        g_W25Q64_Current_Write_Addr += step_len;
        data_ptr += step_len;
        write_len -= step_len;
        remain_bytes_in_page = W25Q64_PAGE_SIZE;

        if (W25Q64_GetSectorStartAddr(g_W25Q64_Current_Write_Addr) != current_sector)
        {
            g_W25Q64_Current_Sector_Erased = 0;
            current_sector = W25Q64_GetSectorStartAddr(g_W25Q64_Current_Write_Addr);
        }
    }

    printf1("Offline saved: %s\r\n", data);
}

/**
 * @brief  任务主函数（只改了else部分）
 */
void Outline_Entry(void *argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xTaskPeriod = pdMS_TO_TICKS(5000);
    char data_buf[32] = {0};

    W25Q64_GPIO_Init();
    W25Q64_SPI1_Init();
    printf1("Outline Task Start\r\n");

    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);

        // ====================== WiFi 离线：存数据 ======================
        if (g_WiFi_Connect_State == 0)
        {
            float light = Get_Light_Intensity();
            float temp  = Get_Temperature();
            uint16_t len = FormatSensorData(light, temp, data_buf);
            Outline_SaveDataToW25Q64(data_buf, len);
            g_WiFi_BT_Task_RunFlag = 1;
        }
        // ====================== WiFi 在线：上传并销毁任务 ======================
        else
        {
            printf1("WiFi Online! Start upload offline data...\r\n");

            // 1. 上传所有离线数据
            if(Outline_UploadToServer() == 1)
            {
                // 2. 上传成功 → 清空Flash
                Outline_ClearFlashData();
            }

            // 3. 销毁自己（任务结束）
            printf1("Outline Suspended\r\n");
            vTaskSuspend(NULL);
        }
    }
}
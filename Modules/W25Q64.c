#include "W25Q64.h"

/*
 * W25Q64 与 STM32 的硬件连接关系如下：W
 *
 * W25Q64_CS   —— PA4  （普通 GPIO 推挽输出，软件片选）
 * W25Q64_DO   —— PA6  （SPI1_MISO，从机输出 / 主机输入）
 * W25Q64_CLK  —— PA5  （SPI1_SCK，时钟信号）
 * W25Q64_DI   —— PA7  （SPI1_MOSI，主机输出 / 从机输入）
 *
 * SPI 外设使用：SPI1
 */

 #define PC13_FLAG_ADDRESS   0x000000
/**
 * @brief  初始化 W25Q64 相关的 GPIO 引脚
 * @param  无
 * @retval 无
 * @note
 *         1. PA5 配置为 SPI1_SCK  —— 复用推挽输出模式
 *         2. PA7 配置为 SPI1_MOSI —— 复用推挽输出模式
 *         3. PA6 配置为 SPI1_MISO —— 浮空输入模式
 *         4. PA4 配置为 W25Q64_CS —— 通用推挽输出模式
 *         5. PA4 引脚默认输出高电平，表示不选中从机
 */
void W25Q64_GPIO_Init(void) {
    // 1. 开启 GPIOA 外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. SPI1_SCK —— PA5，设置为复用推挽输出
    // SPI1_MOSI —— PA7，设置为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // SPI是高速通信协议，选择较快的引脚输出速度，快速翻转电平以满足通信需求
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. SPI1_MISO —— PA6，浮空输入模式
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 5. W25Q64_CS —— PA4，通用推挽输出
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 6. PA4引脚默认输出高电平，默认不选中从机（CS = 高电平）
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


/**
  * @brief  初始化 SPI1 外设，用于与 W25Q64 进行通信
  * @param  无
  * @retval 无
  * @note
  *         1. SPI1 工作在主机模式
  *         2. 数据位宽为 8 bit
  *         3. SPI 模式选择 Mode 0（CPOL=0，CPHA=0）
  *         4. 采用软件方式管理 NSS（SSM=1，SSI=1）
  *         5. SPI 时钟频率为 PCLK2 / 4（默认为 18MHz）
  *         实际上也可以将分频系数选更大一些，SCK时钟频率更大一些
  */
void W25Q64_SPI1_Init(void) {
    // 1. 开启 SPI1 外设时钟（SPI1 挂载在 APB2 总线上）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // 2.设置SPI通信基本参数
    SPI_InitTypeDef SPI_InitStructure;

    // 选择通信方式为SPI全双工，2线模式
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    // 设置STM32为主机模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    // 设置SPI1外设移位寄存器长度是8bit
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    // 时序结构选择SPI Mode 0：空闲时钟为低电平，第一个边沿采样
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    // 比特序选择MSB First，优先发最高有效位
    SPI_InitStructure.SPI_FirstBit  = SPI_FirstBit_MSB;

    // 采用软件NSS
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

    // SPI1时钟线分频系数：当前选择1/4分频
    // 时钟线频率为：72 / 4 = 18MHz
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

    // 3.调用函数，初始化 SPI1 外设
    SPI_Init(SPI1, &SPI_InitStructure);

    // 4.选择软件NSS时，需要额外调用此函数，用于设置SSI为1，代表输入高电平
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);

    // 5. 真正开启 SPI1 外设
    SPI_Cmd(SPI1, ENABLE);
}

/**
  * @brief  SPI1 发送并接收 1 个字节数据
  * @param  data：待发送的数据
  * @retval 接收到的数据
  * @note
  *         SPI 为全双工通信：
  *         发送 1 个字节的同时，必然会接收 1 个字节
  */
static uint8_t SPI1_TransferByte(uint8_t data) {
    // 1. 等待发送数据寄存器为空（TXE = 1）
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    // 2. 待发送1个字节数据写入发送数据寄存器，主机发送1个字节数据给从机
    SPI_I2S_SendData(SPI1, data);

    // 3. 等待接收数据寄存器非空（RXNE = 1）
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    // 4. 读接收数据寄存器，获取从机发送的1个字节数据
    return SPI_I2S_ReceiveData(SPI1);
}

/**
 * @brief  选中 W25Q64（CS = 0）
 * @param  无
 * @retval 无
 */
static void W25Q64_Select(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

/**
 * @brief  取消选中 W25Q64（CS = 1）
 * @param  无
 * @retval 无
 */
static void W25Q64_Deselect(void) {
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**
 * @brief  读取 W25Q64 的 JEDEC ID
 * @param  *ManufacturerID：获取厂商 ID
 * @param  *MemoryType    ：获取存储类型
 * @param  *Capacity      ：获取容量代码
 * @retval 无
 */
void W25Q64_ReadJEDECID(uint8_t *ManufacturerID,
                        uint8_t *MemoryType,
                        uint8_t *Capacity) {
    // 1. 选中 W25Q64，从机片选拉低
    W25Q64_Select();

    // 2. 发送 JEDEC ID 指令 0x9F（接收数据无意义，返回值无需接收）
    SPI1_TransferByte(0x9F);

    // 3. 连续发送3个无意义字节，读取W25Q64返回的 3 个字节
    // 0xFF位置可以填任意1字节数据
    *ManufacturerID = SPI1_TransferByte(0xFF);
    *MemoryType = SPI1_TransferByte(0xFF);
    *Capacity = SPI1_TransferByte(0xFF);

    // 4. 取消片选，结束本次 SPI 通信
    W25Q64_Deselect();
}

/**
 * @brief  读取 W25Q64 状态寄存器1（SR1）
 * @param  无
 * @retval SR1 寄存器的状态值，其中bit0是BUSY标志位的值
 */
static uint8_t W25Q64_ReadStatusRegister1(void) {
    uint8_t Status;

    // 1. 选中从机
    W25Q64_Select();

    // 2. 发送读状态寄存器1指令：0x05
    SPI1_TransferByte(0x05);

    // 3. 发送空字节，读取SR1的返回值
    Status = SPI1_TransferByte(0xFF);

    // 4. 取消片选
    W25Q64_Deselect();

    return Status;
}

/**
 * @brief  等待 W25Q64 空闲（等待 BUSY = 0）
 * @param  无
 * @retval 无
 */
static void W25Q64_WaitBusy(void) {
    uint8_t Status;

    // SR1 的 bit0 是 BUSY 位：1表示忙，0表示空闲
    do {
        Status = W25Q64_ReadStatusRegister1();
    } while ((Status & 0x01) == 0x01);  // bit0位是1，表示仍然处于忙碌状态
}

/**
 * @brief  写使能（Write Enable）
 * @param  无
 * @retval 无
 */
static void W25Q64_WriteEnable(void) {
    // 1. 等待芯片空闲
    W25Q64_WaitBusy();

    // 2. 选中从机
    W25Q64_Select();

    // 3. 发送写使能指令 0x06
    SPI1_TransferByte(0x06);

    // 4. 取消片选
    W25Q64_Deselect();
}

/**
 * @brief  擦除指定扇区（4KB）
 * @param  Address 待擦除扇区的3字节起始地址
 * @retval 无
 * @note SPI采用MSB First比特序，所以3字节起始地址要从最高字节发到最低字节
 */
void W25Q64_SectorErase(uint32_t Address) {
    // 1. 等待芯片空闲
    W25Q64_WaitBusy();

    // 2. 写使能
    W25Q64_WriteEnable();

    // 3. 选中从机
    W25Q64_Select();

    // 4. 发送扇区擦除指令（4KB 扇区）
    SPI1_TransferByte(0x20);

    // 5. 发送 24 位地址
    SPI1_TransferByte((Address >> 16) & 0xFF);  // 发送最高1个字节
    SPI1_TransferByte((Address >> 8)  & 0xFF);  // 发送倒数第二高1个字节
    SPI1_TransferByte(Address & 0xFF);  // 发送最低1个字节

    // 6. 取消片选，让芯片开始执行擦除操作
    W25Q64_Deselect();

    // 7. 等待擦除完成
    W25Q64_WaitBusy();
}

/**
 * @brief  页编程（写多个字节）
 * @param  Address 写入的3字节起始地址
 * @param  Bytes   指向待写入数据的指针（通常是一个字节数组）
 * @param  Length  写入数据长度（1~256，且需要避免跨页）
 * @retval 无
 */
void W25Q64_WriteBytes(uint32_t Address,
                       uint8_t *Bytes,
                       uint16_t Length) {
    // 1. 等待芯片空闲
    W25Q64_WaitBusy();

    // 2. 写使能
    W25Q64_WriteEnable();

    // 3. 选中从机
    W25Q64_Select();

    // 4. 发送页编程指令
    SPI1_TransferByte(0x02);

    // 5. 发送 24 位地址（高字节先发）
    SPI1_TransferByte((Address >> 16) & 0xFF);
    SPI1_TransferByte((Address >> 8)  & 0xFF);
    SPI1_TransferByte(Address & 0xFF);

    // 7. 将待发送数据逐字节发送出去，先发送的字节会先写入芯片起始位置
    for (uint16_t i = 0; i < Length; i++) {
        SPI1_TransferByte(Bytes[i]);
    }

    // 8. 取消片选，芯片开始写入数据
    W25Q64_Deselect();

    // 9. 等待写入完成
    W25Q64_WaitBusy();
}

/**
 * @brief  页编程（写1个字节）
 * @param  Address 写入的3字节起始地址
 * @param  Byte    待写入的1个字节数据
 * @retval 无
 */
void W25Q64_WriteByte(uint32_t Address, uint8_t Byte) {
    // 直接调用已实现函数，完成1个字节数据写入
    W25Q64_WriteBytes(Address, &Byte, 1);
}

/**
 * @brief  从 W25Q64 中读取 1 个字节数据
 * @param  Address 读取地址
 * @retval 读取到的 1 个字节数据
 */
uint8_t W25Q64_ReadByte(uint32_t Address) {

    // 1. 选中从机，开始一次 SPI 通信
    W25Q64_Select();

    // 2. 发送读数据指令 0x03
    SPI1_TransferByte(0x03);

    // 3. 发送 24 位地址（高字节先发）
    SPI1_TransferByte((Address >> 16) & 0xFF);
    SPI1_TransferByte((Address >> 8)  & 0xFF);
    SPI1_TransferByte(Address & 0xFF);

    // 4. 发送无效字节，换取从机返回的 1 个字节数据
    uint8_t Data = SPI1_TransferByte(0xFF);

    // 5. 取消片选，结束本次 SPI 通信
    W25Q64_Deselect();

    return Data;
}

/**
 * @brief  在 W25Q64 中存储 PC13 的状态（断电保持）
 * @param  State PC13 的逻辑状态（枚举值）
 * @retval 无
 * @note   写入前必须擦除；擦除以扇区为单位（4KB），因此需要先对齐扇区首地址
 */
void W25Q64_SavePC13State(PC13State State) {
    // 1. 擦除所在扇区（4KB）
    W25Q64_SectorErase(PC13_FLAG_ADDRESS);

    // 2. 写入 1 个字节状态值
    W25Q64_WriteByte(PC13_FLAG_ADDRESS, (uint8_t)State);
}


/**
 * @brief  从 W25Q64 中读2取 PC13 状态，并还原 PC13 指示灯状态
 * @param  无
 * @retval 返回读取到的 PC13 状态（枚举值）
 */
PC13State W25Q64_RestorePC13State(void) {
    // 1.读取存储的PC13状态字节
    uint8_t SavedPC13State = W25Q64_ReadByte(PC13_FLAG_ADDRESS);

    // 2.还原PC13状态
    if ((PC13State)SavedPC13State == PC13_STATE_ON) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);   // 点亮
    } else {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);     // 熄灭
    }
    return (PC13State)SavedPC13State;
}

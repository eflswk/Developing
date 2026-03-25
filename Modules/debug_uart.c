/* debug_uart.c
 *
 * 模块定位：
 * ------------------------------------------------------------
 * 本文件实现一个基于 USART1 的调试输出工具，
 * 用于在 FreeRTOS 实验中观察任务的创建、运行与调度行为。
 *
 * 设计原则：
 *  1) 对外仅暴露 printf1 接口
 *  2) 底层串口发送细节完全封装
 *  3) 采用阻塞式发送，逻辑直观，便于教学理解
 */

#include "debug_uart.h"

/* ================= 内部函数（不对外暴露） ================= */

/**
 * @brief  USART1 发送单字节（阻塞式）
 * @note   仅供本文件内部使用
 */
static void DebugUSART1_SendByte(uint8_t Byte) {
    /* 等待发送数据寄存器为空 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    USART_SendData(USART1, Byte);
}

/* ================= 对外接口实现 ================= */

/**
 * @brief  初始化 USART1，用于调试输出
 */
void DebugUART_Init(void) {
    /* 1. 开启 USART1 和 GPIOA 外设时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* 2. 初始化 GPIO 引脚 */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* PA9 -> USART1_TX：复用推挽输出 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 -> USART1_RX：上拉输入 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. USART 参数配置：115200, 8N1 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    /* 4. 使能 USART1 */
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief  调试打印函数（串口版 printf）
 * @param  format: 格式化字符串，和printf函数的第一个参数完全一致
 * @note
 *         1. 用法与 printf 类似
 *         2. 实际输出通过 USART1 完成
 *         3. 用于 FreeRTOS 任务调试与运行观测
 */
void printf1(const char *format, ...) {
    // 初始情况下，单次发送字符串最大长度为99
    // 后续可以根据实际情况，增加buffer的容量
    char buffer[100];

    /*
    	参照C语言库函数中printf函数的实现
    	把 printf1("xxx", 参数...) 里的可变参数
    	安全地取出来，并按格式拼成一段字符串。

    	具体的实现，大家可以不了解。
    */
    va_list list;
    va_start(list, format);
    vsprintf(buffer, format, list);
    va_end(list);

    /* 逐字节输出数据到串口，并简单处理Windows端的\n换行 */
    for (uint16_t i = 0; buffer[i] != '\0'; i++) {
        DebugUSART1_SendByte((uint8_t)buffer[i]);
    }
}

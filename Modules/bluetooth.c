#include "bluetooth.h"

/**
  * @brief  初始化蓝牙模块使用的 USART3
  *         默认配置：9600，8位数据位，无校验，1位停止位（8N1）
  * @param  无
  * @retval 无
  */
void BlueTooth_Init(void) {
    // 开启 GPIOB 和 USART3 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 配置 PB10 为 USART3_TX（复用推挽输出）
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 PB11 为 USART3_RX（浮空输入）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 USART3 参数：9600 8N1
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStruct);

    // 开启接收中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 配置 USART3 中断
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // 采用优先级分组4，抢占优先级设置为12
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 12;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    // 使能 USART3
    USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void) {

    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        uint8_t RecvByte = (uint8_t)USART_ReceiveData(USART3);
        /* 中断中只负责把 1 个字节送入队列 */
        xQueueSendFromISR(BT_MsgQueue, &RecvByte, NULL);
    }
}

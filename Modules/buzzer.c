#include "buzzer.h"

void Buzzer_Init(void){
    /* 开启 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    /* PB0 配置为推挽输出 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 默认关闭蜂鸣器，PB0 输出低电平 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}

void Buzzer_On(void){
    /* 打开蜂鸣器，PB0 输出高电平 */
    GPIO_SetBits(GPIOB, GPIO_Pin_15);
}

void Buzzer_Off(void){
    /* 关闭蜂鸣器，PB0 输出低电平 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}

void Buzzer_Beep_C4(uint16_t duration_ms){
    
    uint16_t DelayTime = 500; 
    uint16_t Step = 10;      
    uint16_t MinDelay = 10;  

    while (1) {
        if (DelayTime == MinDelay) {
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            continue;
        }
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
       // Delay_Ms(DelayTime);
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
       // Delay_Ms(DelayTime);
        DelayTime -= Step;
    }
}

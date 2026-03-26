#include "reset_button.h"

/**
 * @brief  初始化按键GPIO（上拉输入）
 */
void Key_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOA时钟
    
    GPIO_InitStruct.GPIO_Pin = RESET_KEY_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RESET_KEY_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief  检测按键是否长按
 * @retval 0：未长按；1：长按
 */
uint8_t Key_Check_LongPress(void)
{
    uint32_t press_time = 0;
    
    // 检测GPIO是否为低电平（按键按下）
    while(GPIO_ReadInputDataBit(RESET_KEY_GPIO_PORT, RESET_KEY_GPIO_PIN) == RESET)
    {
        Delay_Ms(1);
        press_time++;
        if(press_time >= LONG_PRESS_TIME)
        {
            return 1; // 长按触发
        }
    }
    return 0; // 未长按
}

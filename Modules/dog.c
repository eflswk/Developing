#include "dog.h"

/**
 * @brief  检测系统复位原因，通过串口给用户提示
 * @param  无
 * @retval 无
 */
void System_Reset_Check(void)
{
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) 
    {
        // IWDG复位：提示用户"程序异常，看门狗触发复位"
        printf1("[Warning] System reset by IWDG timeout! Check if program is stuck \r\n");
        RCC_ClearFlag(); // 清除复位标志
    }
    else if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
    {
        // WWDG复位：提示用户"喂狗时序异常"
        printf1("[Warning] System reset by WWDG feed violation! Check feed timing \r\n");
        RCC_ClearFlag();
    }
    else
    {
        // 正常启动：提示用户"系统正常上电"
        printf1("[Info] System started normally, no watchdog reset record \r\n");
    }
}

/**
 * @brief  独立看门狗(IWDG)初始化配置
 * @param  无
 * @retval 无
 * @note   超时时间计算：40KHz/4=10KHz → 100us/节拍 → 4095*100us=409.5ms
 */
void IWDG_Config(void)
{
    // 1. 解锁：允许修改预分频和重装载值
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    // 2. 设置预分频系数
    IWDG_SetPrescaler(IWDG_PRESCALER);
    
    // 3. 设置重装载值
    IWDG_SetReload(IWDG_RELOAD_VAL);
    
    // 4. 等待寄存器更新完成（可选，部分芯片需等待）
    IWDG_ReloadCounter();
    
    // 5. 启动IWDG（一旦启动无法关闭，仅复位可停止）
    IWDG_Enable();
    
    // 提示用户：看门狗已启动
    printf1("[Info] Independent watchdog initialized\r\n");
}

/**
 * @brief  窗口看门狗(WWDG)初始化配置（可选扩展）
 * @param  无
 * @retval 无
 * @note   超时时间≈58ms，窗口值0x50：喂狗需在计数器递减到0x50~0x40之间
 */
void WWDG_Config(void)
{
    // 1. 使能WWDG时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    
    // 2. 设置预分频：8分频（基础4096分频+8分频）
    WWDG_SetPrescaler(WWDG_Prescaler_8);
    
    // 3. 设置窗口值：0x50（喂狗不能早于该值）
    WWDG_SetWindowValue(0x50);
    
    // 4. 使能早期唤醒中断（提前提醒喂狗）
    WWDG_EnableIT();
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = WWDG_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    // 5. 启动WWDG，计数器初值0x7F
    WWDG_Enable(0x7F);
    
    // 提示用户：窗口看门狗已启动
    //printf1("[Info] Window watchdog initialized, timeout about 58ms, window value 0x50 \r\n");
}

/**
 * @brief  窗口看门狗中断服务函数（早期唤醒中断）
 * @param  无
 * @retval 无
 * @note   计数器递减到0x40时触发，可在此处喂狗+提示用户
 */
void WWDG_IRQHandler(void)
{
    if (WWDG_GetFlagStatus() != RESET) 
    {
        // 提示用户：即将超时，已在中断喂狗
        printf1("[Warning] WWDG is about to timeout! Feed dog executed in interrupt \r\n");
        
        // 喂狗：重置计数器为0x7F
        WWDG_SetCounter(0x7F);
        
        // 清除中断标志
        WWDG_ClearFlag();
    }
}

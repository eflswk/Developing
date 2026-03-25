#include "delay.h"

/*
 * 函数名：Delay_Init
 * 功  能：初始化 TIM2，用于实现微秒级延时
 *
 * 实现原理：
 *      我们利用 TIM2 的计数器作为“时间计数器”。
 *      系统主频：SYSCLK = 72 MHz
 *      APB1 默认分频为：APB1 = 36 MHz
 *      但需要注意：当 APB1 分频 ≠ 1 时，定时器时钟会自动 ×2
 *      因此：
 *          TIM2 输入时钟 = 36 MHz × 2 = 72 MHz
 *      然后通过设置预分频器 PSC：
 *          CK_CNT = TIM_CLK / (PSC + 1)
 *      设置：
 *          PSC = 71
 *      则：
 *          CK_CNT = 72 MHz / 72 = 1 MHz
 *      也就是：
 *          定时器每计数一次
 *          时间正好经过 1 微秒
 *      因此：
 *          CNT 的值
 *          就等价于经过的微秒数
 */
void Delay_Init(void) {
    /* 1. 使能 TIM2 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /*
     * 开始初始化TIM2外设，也就是给定时器时基结构体赋值
     *
     * 2. 配置 TIM2 的计数周期
     * TIM_Period 是自动重装寄存器 ARR 的值
     * 这里设置为最大值 0xFFFF
     * 这样计数器最大可以计数到 65535
     * 由于：
     *      1 个计数 = 1us
     * 因此最大时间约为：
     *      65535 us ≈ 65 ms
     * 这对于微秒级延时来说已经完全足够
     */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;

    /*
     * 3. 设置预分频器
     * CK_CNT = TIM_CLK / (PSC + 1)
     * TIM_CLK = 72 MHz
     * PSC = 71
     * CK_CNT = 72 MHz / 72 = 1 MHz
     *
     * 因此：
     * 每计数一次 = 1 us
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 71;

    /* 时钟分割因子（影响定时器输入相关的操作，如果只是做延时可以设置为默认） */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    /* 设置为向上计数模式，累加 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    /* 4. 完成初始化定时器 */
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* 5. 启动TIM2定时器 */
    TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  微秒级延时函数（基于 TIM2 硬件计数）
  * @param  us 延时时间，单位：微秒（取值范围：1 ~ 65535）
  *
  * @note   实现原理：
  *         1. TIM2 已配置为 1MHz 计数频率（即 CNT 每 1us 加 1）
  *         2. 将 CNT 清零，从 0 开始计数
  *         3. 当 CNT 计数达到 us 时，表示已经经过 us 微秒
  *
  * @note   注意事项：
  *         1. 该函数为“阻塞式延时”，执行期间 CPU 会一直空转等待
  *         2. 最大延时受 ARR 限制（本例为 65535us ≈ 65.5ms）
  *         3. 不建议在中断或 RTOS 任务中长时间使用
  *         4. 实际延时会存在极小误差（函数执行开销导致）
  */
void Delay_Us(uint16_t us){
	TIM_SetCounter(TIM2, 0);
	while(TIM_GetCounter(TIM2) < us);
}


/**
  * @brief  毫秒级延时函数
  * @param  ms 延时时间，单位：毫秒
  */
void Delay_Ms(uint16_t ms) {
    while (ms--) {
        Delay_Us(1000);
    }
}

/**
  * @brief  秒级延时函数
  * @param  s 延时时间，单位：秒
  */
void Delay_S(uint16_t s) {
    while (s--) {
        Delay_Ms(1000);
    }
}

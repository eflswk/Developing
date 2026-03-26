#include "ADC.h"

/**
 * @brief  配置 ADC1 以进行模数转换
 * @note   该函数初始化 ADC1，并配置其转换模式、数据对齐方式等
 * @param  无
 * @retval 无
 */
void ADC_Config(void) {
    // 配置 ADC 时钟，确保低于 14MHz
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 设置 ADC 时钟 = APB2 时钟 / 6 = 12MHz（恰好合适）

    // 开启 ADC1 和 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // GPIO 配置模拟信号输入引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;  // 选择 PA0 和 PA1 作为 ADC 输入通道
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 配置为模拟输入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 初始化 GPIOA

    // ADC 配置
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式，不与其他 ADC 共享
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;  // 关闭扫描模式（同时只有一个单通道进行转换）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // 关闭连续转换模式（手动触发转换）
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 不使用外部触发，采用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  // 结果右对齐（低位对齐方式）
    ADC_InitStructure.ADC_NbrOfChannel = 1;  // 只配置 1 个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);  // 初始化 ADC1

    // 开启 ADC1
    ADC_Cmd(ADC1, ENABLE);

    // 复位 ADC1 校准
    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1));  // 等待复位完成

    // 开始 ADC1 校准
    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1));  // 等待校准完成
}

/**
 * @brief  读取指定通道的 ADC 值
 * @note   该函数启动 ADC1 进行单次转换，并返回转换结果
 * @param  channel: 要转换的 ADC 通道号（ADC_Channel_0 ~ ADC_Channel_17）
 * @retval uint16_t: 12 位 ADC 转换结果（范围 0 ~ 4095）
 */
uint16_t ADC_Read(uint8_t channel) {
    // 配置 ADC 通道（指定通道，规则组序号 1，采样时间 55.5 周期）
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);

    // 开始 ADC1采样,转换数据的过程（软件触发）
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 等待转换完成（检查 EOC 标志, 等待该标志被置为1）
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    // 读取并返回转换结果, 自动将EOC标志位置为0
    return ADC_GetConversionValue(ADC1);
}

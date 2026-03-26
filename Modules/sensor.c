#include "sensor.h"

/**
 * @brief  获取光照强度（单位：lux）
 * @note   根据资料，光敏模块采用 5516 光敏电阻，并与 10kΩ 电阻构成分压电路。
 *         先根据 ADC 值计算电压，再反推出光敏电阻阻值，最后估算光照强度。
 *         这里采用 5516 光敏电阻的典型参数进行估算，结果为近似值。
 * @retval float 光照强度（单位：lux）
 */
float Get_Light_Intensity(void) {
    uint16_t adc_value = ADC_Read(ADC_Channel_1);

    float voltage = (adc_value / 4095.0f) * 3.3f;

    if (voltage <= 0.001f) {
        return 65535.0f;
    }

    if (voltage >= 3.299f) {
        return 0.0f;
    }

    return powf((12000.0f * powf(10.0f, 0.8f)) / ((10000.0f * voltage) / (3.3f - voltage)), 1.0f / 0.8f);
}

/**
 * @brief  计算温度（单位：°C）
 * @note   采用 Steinhart-Hart 公式计算热敏电阻温度，适用于 NTC 热敏电阻。
 *         热敏电阻与一个固定电阻（10kΩ）构成分压电路，
 *         需要先计算热敏电阻的电阻值，再用公式求温度。
 * @retval float 温度值（单位：°C）
 */
float Get_Temperature(void) {
    // 读取 ADC 值 (0 ~ 4095)，对应热敏电阻的电压分压值
    uint16_t adc_value = ADC_Read(ADC_Channel_0); // 读取 PA0（热敏电阻传感器）

    // 将 ADC 数值转换为电压值（单位：V），参考电压为 3.3V
    float voltage = (adc_value / 4096.0) * 3.3;

    // 计算热敏电阻的实际阻值（分压公式推导）
    float resistance = (10000.0 * voltage) / (3.3 - voltage); // 上拉电阻 10kΩ

    // 使用 Steinhart-Hart 公式计算温度（适用于 10kΩ NTC 热敏电阻）
    // Steinhart-Hart 公式参数（使用 B 参数公式，适用于 10kΩ 热敏电阻）
    float temperature = 1 / (0.003354 + (0.000256985 * log(resistance / 10000.0))) - 273.15;

    return temperature; // 返回计算出的温度值
}

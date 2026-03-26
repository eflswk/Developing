#ifndef SENSOR_H
#define SENSOR_H

#include "stm32f10x.h"
#include "ADC.h"
#include <math.h>

// 通过光敏电阻传感器获取光照强度, 单位Lux
float Get_Light_Intensity(void);
// 通过热敏电阻传感器获取环境温度, 单位摄氏度
float Get_Temperature(void);

#endif // SENSOR_DRIVER_H

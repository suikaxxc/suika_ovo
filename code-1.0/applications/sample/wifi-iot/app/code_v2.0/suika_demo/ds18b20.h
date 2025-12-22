/**
 * @file ds18b20.h
 * @brief DS18B20 temperature sensor interface for aquatic plant tank
 */

#ifndef __DS18B20_H__
#define __DS18B20_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initialize and start DS18B20 temperature sensor task
 */
void DS18B20_MainLoop(void);

/**
 * @brief Get current water temperature in Celsius
 * @return Temperature in Celsius (with one decimal precision)
 */
float Get_WaterTemperature(void);

/**
 * @brief Check if DS18B20 sensor is present
 * @return 1 if present, 0 if not
 */
int DS18B20_IsPresent(void);

#ifdef __cplusplus
}
#endif

#endif /* __DS18B20_H__ */

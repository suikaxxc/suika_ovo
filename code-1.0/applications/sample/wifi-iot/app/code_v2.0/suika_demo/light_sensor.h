/**
 * @file light_sensor.h
 * @brief Light sensor (LDR) interface for aquatic plant tank
 */

#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start light sensor task
 */
void LightSensor_MainLoop(void);

/**
 * @brief Get current light intensity (0-100%)
 * @return Light intensity percentage
 */
int Get_LightIntensity(void);

/**
 * @brief Get raw light sensor ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_LightRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIGHT_SENSOR_H__ */

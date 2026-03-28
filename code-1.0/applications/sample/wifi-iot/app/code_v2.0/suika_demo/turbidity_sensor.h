/**
 * @file turbidity_sensor.h
 * @brief Turbidity sensor interface for aquatic plant tank
 */

#ifndef __TURBIDITY_SENSOR_H__
#define __TURBIDITY_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Collect ADC sample for turbidity median filtering
 */
void Turbidity_CollectSample(void);

/**
 * @brief Update turbidity value (NTU)
 */
void Turbidity_Update(void);

/**
 * @brief Get current turbidity value in NTU
 * @return Turbidity value (0-1000 NTU)
 */
int Get_TurbidityValue(void);

/**
 * @brief Get raw turbidity ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_TurbidityRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __TURBIDITY_SENSOR_H__ */

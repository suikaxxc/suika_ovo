/**
 * @file turbidity_sensor.h
 * @brief AZDM01 turbidity sensor interface for aquatic plant tank
 *
 * Sensor output: analog voltage (typically 0.5V ~ 4.5V)
 * NTU formula: NTU = -125 * Vout + 625
 */

#ifndef __TURBIDITY_SENSOR_H__
#define __TURBIDITY_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize turbidity sensor (no-op, for compatibility)
 */
void Turbidity_MainLoop(void);

/**
 * @brief Update turbidity sensor reading (call periodically)
 */
void Turbidity_Update(void);

/**
 * @brief Get current turbidity in NTU
 * @return NTU value
 */
int Get_TurbidityNTU(void);

/**
 * @brief Get current sensor output voltage (Vout, volts)
 * @return Vout in volts
 */
float Get_TurbidityVoltage(void);

/**
 * @brief Get averaged raw ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_TurbidityRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __TURBIDITY_SENSOR_H__ */

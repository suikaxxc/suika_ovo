/**
 * @file temp_control.h
 * @brief Temperature control interface (heater and fan) for aquatic plant tank
 */

#ifndef __TEMP_CONTROL_H__
#define __TEMP_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize temperature control (heater and fan)
 */
void TempControl_Init(void);

/**
 * @brief Turn on heater
 */
void Heater_On(void);

/**
 * @brief Turn off heater
 */
void Heater_Off(void);

/**
 * @brief Get heater status
 * @return 1 if on, 0 if off
 */
int Heater_GetState(void);

/**
 * @brief Set fan control value (0=off, non-zero=on)
 * @param speedPercent Kept for compatibility; hardware treats non-zero as ON
 */
void Fan_SetSpeed(int speedPercent);

/**
 * @brief Get current fan state value
 * @return 0 for OFF, 1 for ON
 */
int Fan_GetSpeed(void);

/**
 * @brief Stop fan
 */
void Fan_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEMP_CONTROL_H__ */

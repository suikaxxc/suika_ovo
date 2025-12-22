/**
 * @file led_control.h
 * @brief LED plant light control interface for aquatic plant tank
 */

#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LED control
 */
void LED_Init(void);

/**
 * @brief Turn on LED light
 */
void LED_On(void);

/**
 * @brief Turn off LED light
 */
void LED_Off(void);

/**
 * @brief Get LED status
 * @return 1 if on, 0 if off
 */
int LED_GetState(void);

/**
 * @brief Toggle LED state
 */
void LED_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_CONTROL_H__ */

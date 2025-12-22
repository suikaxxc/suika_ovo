/**
 * @file alarm.h
 * @brief Alarm (beeper) control interface for aquatic plant tank
 */

#ifndef __ALARM_H__
#define __ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif

// Alarm levels
typedef enum {
    ALARM_NONE = 0,
    ALARM_WARNING = 1,
    ALARM_DANGER = 2
} AlarmLevel;

/**
 * @brief Initialize alarm (beeper)
 */
void Alarm_Init(void);

/**
 * @brief Trigger alarm with specified level
 * @param level Alarm level
 * @param message Alarm message (for logging)
 */
void Alarm_Trigger(AlarmLevel level, const char *message);

/**
 * @brief Stop alarm
 */
void Alarm_Stop(void);

/**
 * @brief Get current alarm level
 * @return Current alarm level
 */
AlarmLevel Alarm_GetLevel(void);

/**
 * @brief Get alarm message
 * @return Current alarm message
 */
const char* Alarm_GetMessage(void);

/**
 * @brief Beep once for notification
 */
void Alarm_Beep(void);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_H__ */

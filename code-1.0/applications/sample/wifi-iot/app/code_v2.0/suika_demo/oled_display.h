/**
 * @file oled_display.h
 * @brief OLED display interface for aquatic plant tank
 */

#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start OLED display task
 */
void OledDisplay_MainLoop(void);

/**
 * @brief Switch to next OLED display page (manual page flip)
 */
void OledDisplay_NextPage(void);

/**
 * @brief Get current OLED display page number
 * @return Current page (0=Sensors, 1=Actuators, 2=System)
 */
int OledDisplay_GetCurrentPage(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_DISPLAY_H__ */

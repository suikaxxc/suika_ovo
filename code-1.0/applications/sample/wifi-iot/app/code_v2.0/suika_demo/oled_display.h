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

#ifdef __cplusplus
}
#endif

#endif /* __OLED_DISPLAY_H__ */

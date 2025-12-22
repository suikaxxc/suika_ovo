#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>
#include "ohos_types.h"
// #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define OLED_I2C_IDX WIFI_IOT_I2C_IDX_0

#define OLED_WIDTH (128)
#define OLED_I2C_ADDR 0x78             // 默认地址为 0x78
#define OLED_I2C_CMD 0x00              // 0000 0000       写命令
#define OLED_I2C_DATA 0x40             // 0100 0000(0x40) 写数据
#define OLED_I2C_BAUDRATE (400 * 1000) // 400k

#define DELAY_100_MS (100 * 1000)

/**
 * @brief ssd1306 OLED Initialize.
 */
uint32_t OledInit(void);

/**
 * @brief Set cursor position
 *
 * @param x the horizontal posistion of cursor
 * @param y the vertical position of cursor
 * @return Returns {@link WIFI_IOT_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link wifiiot_errno.h} otherwise.
 */
void OledSetPosition(uint8_t x, uint8_t y);

void OledFillScreen(uint8_t fillData);

enum Font
{
    FONT6x8 = 1,
    FONT8x16
};
typedef enum Font Font;

extern void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font);
extern void OledShowString(uint8_t x, uint8_t y, const char *str, Font font);

#endif // OLED_SSD1306_H

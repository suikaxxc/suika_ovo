#include "i2c_common.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

osMutexId_t g_i2cMutex = NULL;

void I2C_CommonInit(void)
{
    if (g_i2cMutex == NULL) {
        osMutexAttr_t attr = {0};
        g_i2cMutex = osMutexNew(&attr);
    }

    // 统一初始化 I2C0
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(WIFI_IOT_I2C_IDX_0, 400000); // 400kHz
}

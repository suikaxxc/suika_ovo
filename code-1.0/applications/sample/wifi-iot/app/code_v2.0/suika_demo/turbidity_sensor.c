/**
 * @file turbidity_sensor.c
 * @brief Turbidity sensor implementation for aquatic plant tank
 * Uses GPIO1 digital input for turbidity threshold detection
 * (compatible with common Arduino turbidity module DO pin logic: LOW triggered)
 */

#include <stdint.h>

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "turbidity_sensor.h"

#define TURBIDITY_SENSOR_GPIO WIFI_IOT_GPIO_IDX_1
#define TURBIDITY_SENSOR_IO   WIFI_IOT_IO_NAME_GPIO_1
#define TURBIDITY_THRESHOLD_CLEAR_NTU 0
#define TURBIDITY_THRESHOLD_DIRTY_NTU 1000
#define TURBIDITY_DIGITAL_LOW_RAW 0
#define TURBIDITY_DIGITAL_HIGH_RAW 4095

static unsigned short g_turbidity_raw = 0;
static int g_turbidity_ntu = 0;
static int g_turbidity_inited = 0;

static void Turbidity_InitDigitalInput(void)
{
    if (g_turbidity_inited)
    {
        return;
    }
    IoSetFunc(TURBIDITY_SENSOR_IO, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    IoSetPull(TURBIDITY_SENSOR_IO, WIFI_IOT_IO_PULL_NONE);
    GpioSetDir(TURBIDITY_SENSOR_GPIO, WIFI_IOT_GPIO_DIR_IN);
    g_turbidity_inited = 1;
}

void Turbidity_CollectSample(void)
{
    Turbidity_InitDigitalInput();

    WifiIotGpioValue pin_value = WIFI_IOT_GPIO_VALUE1;
    if (GpioGetInputVal(TURBIDITY_SENSOR_GPIO, &pin_value) == WIFI_IOT_SUCCESS)
    {
        if (pin_value == WIFI_IOT_GPIO_VALUE0)
        {
            // Arduino reference: LOW means triggered (high turbidity threshold reached)
            g_turbidity_raw = TURBIDITY_DIGITAL_LOW_RAW;
            g_turbidity_ntu = TURBIDITY_THRESHOLD_DIRTY_NTU;
        }
        else
        {
            g_turbidity_raw = TURBIDITY_DIGITAL_HIGH_RAW;
            g_turbidity_ntu = TURBIDITY_THRESHOLD_CLEAR_NTU;
        }
    }
}

void Turbidity_Update(void)
{
    Turbidity_CollectSample();
}

int Get_TurbidityValue(void)
{
    return g_turbidity_ntu;
}

unsigned short Get_TurbidityRaw(void)
{
    return g_turbidity_raw;
}

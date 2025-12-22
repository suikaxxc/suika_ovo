/**
 * @file tds_sensor.c
 * @brief TDS water quality sensor implementation for aquatic plant tank
 * Uses ADC5 (GPIO11) for TDS measurement
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "tds_sensor.h"
#include "ds18b20.h"

#define TDS_TASK_STACK_SIZE 2048
#define TDS_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_5  // GPIO11/ADC5

// TDS sensor calibration parameters
#define TDS_VREF 3.3f
#define TDS_ADC_MAX 4095.0f

static unsigned short g_tds_raw = 0;
static int g_tds_ppm = 0;

int Get_TDSValue(void)
{
    return g_tds_ppm;
}

unsigned short Get_TDSRaw(void)
{
    return g_tds_raw;
}

/**
 * @brief Calculate TDS value from ADC reading with temperature compensation
 * @param adcValue Raw ADC value
 * @param temperature Current water temperature in Celsius
 * @return TDS value in ppm
 */
static int CalculateTDS(unsigned short adcValue, float temperature)
{
    // Convert ADC value to voltage
    float voltage = (float)adcValue * TDS_VREF / TDS_ADC_MAX;

    // Temperature compensation coefficient (standard: 25°C)
    float compensationCoeff = 1.0f + 0.02f * (temperature - 25.0f);
    float compensatedVoltage = voltage / compensationCoeff;

    // Convert voltage to TDS (based on typical TDS sensor calibration)
    // TDS = (133.42 * V^3 - 255.86 * V^2 + 857.39 * V) * 0.5
    float tds = (133.42f * compensatedVoltage * compensatedVoltage * compensatedVoltage
                 - 255.86f * compensatedVoltage * compensatedVoltage
                 + 857.39f * compensatedVoltage) * 0.5f;

    if (tds < 0) tds = 0;
    if (tds > 1000) tds = 1000;

    return (int)tds;
}

static void TDS_Task(void *arg)
{
    (void)arg;
    GpioInit();

    while (1)
    {
        if (AdcRead(TDS_ADC_CHANNEL, &g_tds_raw,
                    WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            // Get current water temperature for compensation
            float temp = Get_WaterTemperature();
            g_tds_ppm = CalculateTDS(g_tds_raw, temp);
            printf("[TDS] raw:%u ppm:%d\n", g_tds_raw, g_tds_ppm);
        }
        sleep(3);
    }
}

void TDS_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "TDS_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TDS_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)TDS_Task, NULL, &attr) == NULL)
    {
        printf("[TDS] Failed to create task!\n");
    }
}

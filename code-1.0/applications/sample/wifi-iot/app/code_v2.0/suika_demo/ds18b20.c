/**
 * @file ds18b20.c
 * @brief DS18B20 temperature sensor implementation for aquatic plant tank
 * Uses GPIO02 for 1-wire communication
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"
#include "hi_time.h"

#include "ds18b20.h"

#define DS18B20_TASK_STACK_SIZE 2048
#define DS18B20_GPIO WIFI_IOT_GPIO_IDX_2
#define DS18B20_IO_NAME WIFI_IOT_IO_NAME_GPIO_2

// DS18B20 Commands
#define DS18B20_CMD_SKIP_ROM    0xCC
#define DS18B20_CMD_CONVERT_T   0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

static float g_water_temp = 25.0f;
static int g_sensor_present = 0;

float Get_WaterTemperature(void)
{
    return g_water_temp;
}

int DS18B20_IsPresent(void)
{
    return g_sensor_present;
}

static void DS18B20_SetOutput(void)
{
    GpioSetDir(DS18B20_GPIO, WIFI_IOT_GPIO_DIR_OUT);
}

static void DS18B20_SetInput(void)
{
    GpioSetDir(DS18B20_GPIO, WIFI_IOT_GPIO_DIR_IN);
}

static void DS18B20_Write(int val)
{
    GpioSetOutputVal(DS18B20_GPIO, val ? WIFI_IOT_GPIO_VALUE1 : WIFI_IOT_GPIO_VALUE0);
}

static int DS18B20_Read(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(DS18B20_GPIO, &val);
    return (val == WIFI_IOT_GPIO_VALUE1) ? 1 : 0;
}

static int DS18B20_Reset(void)
{
    int presence = 0;

    DS18B20_SetOutput();
    DS18B20_Write(0);
    hi_udelay(480);  // Hold low for 480us

    DS18B20_Write(1);
    DS18B20_SetInput();
    hi_udelay(70);   // Wait 70us

    presence = DS18B20_Read() == 0 ? 1 : 0;
    hi_udelay(410);  // Wait for presence pulse to complete

    return presence;
}

static void DS18B20_WriteByte(uint8_t data)
{
    DS18B20_SetOutput();
    for (int i = 0; i < 8; i++)
    {
        DS18B20_Write(0);
        hi_udelay(2);
        if (data & 0x01)
        {
            DS18B20_Write(1);
        }
        hi_udelay(60);
        DS18B20_Write(1);
        hi_udelay(2);
        data >>= 1;
    }
}

static uint8_t DS18B20_ReadByte(void)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        DS18B20_SetOutput();
        DS18B20_Write(0);
        hi_udelay(2);
        DS18B20_Write(1);
        DS18B20_SetInput();
        hi_udelay(10);
        data >>= 1;
        if (DS18B20_Read())
        {
            data |= 0x80;
        }
        hi_udelay(50);
    }
    return data;
}

static float DS18B20_ReadTemperature(void)
{
    uint8_t temp_lsb, temp_msb;
    int16_t temp_raw;

    if (!DS18B20_Reset())
    {
        g_sensor_present = 0;
        return -127.0f;
    }
    g_sensor_present = 1;

    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);

    // Wait for conversion (750ms max for 12-bit)
    usleep(750000);

    if (!DS18B20_Reset())
    {
        return -127.0f;
    }

    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);

    temp_lsb = DS18B20_ReadByte();
    temp_msb = DS18B20_ReadByte();

    temp_raw = (int16_t)((temp_msb << 8) | temp_lsb);
    return (float)temp_raw / 16.0f;
}

static void DS18B20_Task(void *arg)
{
    (void)arg;

    IoSetFunc(DS18B20_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    IoSetPull(DS18B20_IO_NAME, WIFI_IOT_IO_PULL_UP);
    GpioSetDir(DS18B20_GPIO, WIFI_IOT_GPIO_DIR_OUT);

    // Initial sensor check
    sleep(1);  // Wait for GPIO to stabilize
    if (DS18B20_Reset())
    {
        g_sensor_present = 1;
        printf("[DS18B20] Sensor detected\r\n");
    }
    else
    {
        g_sensor_present = 0;
        printf("[DS18B20] Sensor not found\r\n");
    }

    while (1)
    {
        float temp = DS18B20_ReadTemperature();
        if (temp > -100.0f)
        {
            g_water_temp = temp;
        }
        sleep(3);
    }
}

void DS18B20_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "DS18B20_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DS18B20_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)DS18B20_Task, NULL, &attr) == NULL)
    {
        printf("[DS18B20] Failed to create task!\r\n");
    }
}

/**
 * @file main.c
 * @brief Main entry point for Smart Aquatic Plant Tank System
 * 
 * This is the hardware control firmware for a smart water plant cultivation tank
 * based on Hi3861 SoC. It integrates with a HarmonyOS mobile app via MQTT.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

// Module headers
#include "i2c_common.h"
#include "water_level.h"
#include "ds18b20.h"
#include "tds_sensor.h"
#include "light_sensor.h"
#include "tank_control.h"
#include "oled_display.h"
#include "mqtt_client.h"

#define MAIN_TASK_STACK_SIZE 4096

static void AquaticTank_Task(void *arg)
{
    (void)arg;

    // Wait for system to stabilize after boot
    sleep(1);

    printf("\r\n");
    printf("============================================\r\n");
    printf("  Smart Aquatic Plant Tank System v1.0     \r\n");
    printf("  Based on Hi3861 + HarmonyOS              \r\n");
    printf("============================================\r\n");
    printf("\r\n");

    // Initialize tank control system (actuators)
    TankControl_Init();

    // Start sensor tasks (each creates its own thread)
    WaterLevel_MainLoop();    // Water level sensor
    DS18B20_MainLoop();       // Temperature sensor
    TDS_MainLoop();           // TDS water quality sensor
    LightSensor_MainLoop();   // Light sensor

    // Start OLED display task
    OledDisplay_MainLoop();

    // Start control logic task
    TankControl_MainLoop();

    // Start MQTT communication task (includes WiFi connection)
    MQTT_MainLoop();

    printf("[Main] All tasks started successfully\r\n");
}

static void MainTask(void)
{
    osThreadAttr_t attr;
    attr.name = "AquaticTank_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    // Initialize I2C0 with mutex for OLED display
    I2C_CommonInit();

    if (osThreadNew((osThreadFunc_t)AquaticTank_Task, NULL, &attr) == NULL)
    {
        printf("[Main] Failed to create AquaticTank_Task!\r\n");
    }
}

APP_FEATURE_INIT(MainTask);

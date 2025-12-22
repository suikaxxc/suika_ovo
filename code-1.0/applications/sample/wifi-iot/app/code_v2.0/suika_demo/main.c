/**
 * @file main.c
 * @brief Main entry point for Smart Aquatic Plant Tank System
 * 
 * This is the hardware control firmware for a smart water plant cultivation tank
 * based on Hi3861 SoC. It integrates with a HarmonyOS mobile app via MQTT.
 * 
 * Features:
 * 1. Smart auto water replenishment (water level control)
 * 2. Smart temperature control (heater + PWM fan)
 * 3. Smart LED lighting (based on ambient light)
 * 4. Alarm system (beeper for safety alerts)
 * 5. WiFi connectivity with MQTT communication
 * 6. Remote control via HarmonyOS app
 * 7. AI-powered plant care parameters
 * 
 * Sensors:
 * - YW001 water level sensor (ADC3/GPIO07)
 * - DS18B20 water temperature sensor (GPIO02)
 * - TDS water quality sensor (ADC5/GPIO11)
 * - LDR light sensor (ADC0/GPIO12)
 * 
 * Actuators:
 * - L9110S motor driver #1 for drain pump (GPIO00, GPIO01)
 * - L9110S motor driver #2 for fill pump (GPIO06, GPIO08)
 * - Heater (GPIO10)
 * - PWM fan (GPIO04/PWM1)
 * - LED plant light (GPIO03)
 * - Beeper alarm (GPIO09/PWM0)
 * 
 * Display:
 * - OLED 0.96" (I2C0: SDA=GPIO13, SCL=GPIO14)
 * - S1/S2 buttons for page switching and mode toggle
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

    printf("\n");
    printf("============================================\n");
    printf("  Smart Aquatic Plant Tank System v1.0     \n");
    printf("  Based on Hi3861 + HarmonyOS              \n");
    printf("============================================\n");
    printf("\n");

    // Initialize tank control system (actuators)
    TankControl_Init();

    // Start sensor tasks
    WaterLevel_MainLoop();    // Water level sensor
    DS18B20_MainLoop();       // Temperature sensor
    TDS_MainLoop();           // TDS water quality sensor
    LightSensor_MainLoop();   // Light sensor

    // Wait for sensors to get initial readings
    sleep(3);

    // Start control logic task
    TankControl_MainLoop();

    // Start OLED display task
    OledDisplay_MainLoop();

    // Start MQTT communication task
    MQTT_MainLoop();

    printf("[Main] All tasks started successfully\n");
    printf("[Main] System is now running in AUTO mode\n");
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
        printf("[Main] Failed to create AquaticTank_Task!\n");
    }
}

APP_FEATURE_INIT(MainTask);

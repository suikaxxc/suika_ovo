/**
 * @file main.c
 * @brief Main entry point for Smart Aquatic Plant Tank System
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

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
    
    // Wait for system to stabilize
    sleep(1);
    
    printf("Smart Aquatic Tank System Starting...\n");

    // Initialize tank control system (actuators)
    TankControl_Init();

    // Start OLED display task
    OledDisplay_MainLoop();

    // Start control logic task (includes sensor polling)
    TankControl_MainLoop();

    // Start MQTT communication task
    MQTT_MainLoop();

    printf("All tasks started\n");
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
        printf("Failed to create AquaticTank_Task!\n");
    }
}

APP_FEATURE_INIT(MainTask);

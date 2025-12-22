/**
 * @file oled_display.c
 * @brief OLED display implementation for aquatic plant tank
 * Shows sensor data and actuator status on OLED screen
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "oled_display.h"
#include "oled_ssd1306.h"
#include "i2c_common.h"
#include "water_level.h"
#include "ds18b20.h"
#include "tds_sensor.h"
#include "light_sensor.h"
#include "pump_control.h"
#include "temp_control.h"
#include "led_control.h"
#include "alarm.h"
#include "tank_control.h"
#include "wifi_connect.h"
#include "mqtt_client.h"

#define OLED_TASK_STACK_SIZE 4096

// Display pages
#define PAGE_SENSORS 0
#define PAGE_ACTUATORS 1
#define PAGE_STATUS 2
#define PAGE_COUNT 3

// Button ADC channel (GPIO05/ADC2 - same as display board)
#define BTN_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_2

static int g_current_page = PAGE_SENSORS;

static void RenderSensorPage(char *line, size_t lineSize)
{
    int waterLevel = Get_WaterLevelPercent();
    float waterTemp = Get_WaterTemperature();
    int tdsValue = Get_TDSValue();
    int lightIntensity = Get_LightIntensity();
    const TankParams *params = TankControl_GetParams();

    // Line 1: Water Level
    snprintf(line, lineSize, "WL:%d%%(%d-%d)", waterLevel,
             params->waterLevelMin, params->waterLevelMax);
    OledShowString(0, 1, line, 1);

    // Line 2: Water Temperature
    snprintf(line, lineSize, "WT:%.1fC(%.0f-%.0f)", waterTemp,
             params->waterTempMin, params->waterTempMax);
    OledShowString(0, 2, line, 1);

    // Line 3: TDS
    snprintf(line, lineSize, "TDS:%dppm", tdsValue);
    OledShowString(0, 3, line, 1);

    // Line 4: Light
    snprintf(line, lineSize, "Light:%d%%(%d)", lightIntensity,
             params->lightThreshold);
    OledShowString(0, 4, line, 1);

    // Line 5: Alarm status
    AlarmLevel alarm = Alarm_GetLevel();
    if (alarm == ALARM_NONE)
    {
        OledShowString(0, 5, "Status: OK", 1);
    }
    else if (alarm == ALARM_WARNING)
    {
        OledShowString(0, 5, "Status: WARN", 1);
    }
    else
    {
        OledShowString(0, 5, "Status: DANGER", 1);
    }
}

static void RenderActuatorPage(char *line, size_t lineSize)
{
    // Line 1: Fill Pump
    snprintf(line, lineSize, "Fill Pump: %s",
             Pump_GetState(PUMP_FILL) ? "ON " : "OFF");
    OledShowString(0, 1, line, 1);

    // Line 2: Drain Pump
    snprintf(line, lineSize, "Drain Pump: %s",
             Pump_GetState(PUMP_DRAIN) ? "ON " : "OFF");
    OledShowString(0, 2, line, 1);

    // Line 3: Heater
    snprintf(line, lineSize, "Heater: %s",
             Heater_GetState() ? "ON " : "OFF");
    OledShowString(0, 3, line, 1);

    // Line 4: Fan
    snprintf(line, lineSize, "Fan: %d%%", Fan_GetSpeed());
    OledShowString(0, 4, line, 1);

    // Line 5: LED
    snprintf(line, lineSize, "LED: %s",
             LED_GetState() ? "ON " : "OFF");
    OledShowString(0, 5, line, 1);
}

static void RenderStatusPage(char *line, size_t lineSize)
{
    // Line 1: Mode
    snprintf(line, lineSize, "Mode: %s",
             TankControl_GetMode() == CONTROL_MODE_AUTO ? "AUTO  " : "MANUAL");
    OledShowString(0, 1, line, 1);

    // Line 2: WiFi
    snprintf(line, lineSize, "WiFi: %s",
             WiFi_IsConnected() ? "Connected" : "Disconn.  ");
    OledShowString(0, 2, line, 1);

    // Line 3: MQTT
    snprintf(line, lineSize, "MQTT: %s",
             MQTT_IsConnected() ? "Connected" : "Disconn.  ");
    OledShowString(0, 3, line, 1);

    // Line 4: DS18B20 sensor
    snprintf(line, lineSize, "TempSensor: %s",
             DS18B20_IsPresent() ? "OK" : "N/A");
    OledShowString(0, 4, line, 1);

    // Line 5: Alarm message (truncated)
    const char *alarmMsg = Alarm_GetMessage();
    if (alarmMsg[0] != '\0')
    {
        char msgBuf[17];  // 16 chars + null
        strncpy(msgBuf, alarmMsg, 16);
        msgBuf[16] = '\0';
        OledShowString(0, 5, msgBuf, 1);
    }
    else
    {
        OledShowString(0, 5, "No Alarm        ", 1);
    }
}

static void OledDisplay_Task(void *arg)
{
    (void)arg;
    static char line[32] = {0};
    int lastBtnState = 0;

    // Initialize OLED
    GpioInit();
    OledInit();
    OledFillScreen(0x00);
    OledShowString(0, 0, "Aquatic Tank", 1);
    sleep(2);

    // Initialize button ADC
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_5, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);

    while (1)
    {
        unsigned short adc_val = 0;
        int currentBtnState = 0;

        // Read button ADC
        if (AdcRead(BTN_ADC_CHANNEL, &adc_val,
                    WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            if (adc_val < 200)
            {
                currentBtnState = 1;  // S1 pressed
            }
            else if (adc_val > 400 && adc_val < 1200)
            {
                currentBtnState = 2;  // S2 pressed
            }
            else
            {
                currentBtnState = 0;  // None
            }
        }

        // Handle button press (rising edge)
        if (lastBtnState == 0 && currentBtnState != 0)
        {
            if (currentBtnState == 1)
            {
                // S1: Switch page
                g_current_page = (g_current_page + 1) % PAGE_COUNT;
                OledFillScreen(0x00);

                const char *titles[] = {"[Sensors]", "[Actuators]", "[System]"};
                OledShowString(0, 0, titles[g_current_page], 1);
                printf("[OLED] Switched to page %d\n", g_current_page);
            }
            else if (currentBtnState == 2)
            {
                // S2: Toggle mode
                if (TankControl_GetMode() == CONTROL_MODE_AUTO)
                {
                    TankControl_SetMode(CONTROL_MODE_MANUAL);
                }
                else
                {
                    TankControl_SetMode(CONTROL_MODE_AUTO);
                }
                printf("[OLED] Mode toggled\n");
            }
        }
        lastBtnState = currentBtnState;

        // Render current page
        switch (g_current_page)
        {
            case PAGE_SENSORS:
                RenderSensorPage(line, sizeof(line));
                break;
            case PAGE_ACTUATORS:
                RenderActuatorPage(line, sizeof(line));
                break;
            case PAGE_STATUS:
                RenderStatusPage(line, sizeof(line));
                break;
        }

        usleep(300000);  // 300ms refresh
    }
}

void OledDisplay_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "OledDisplay_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = OLED_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)OledDisplay_Task, NULL, &attr) == NULL)
    {
        printf("[OledDisplay] Failed to create task!\n");
    }
}

/**
 * @file oled_display.c
 * @brief OLED display implementation for aquatic plant tank
 * Shows sensor data and actuator status on OLED screen
 * 
 * Note: GPIO05 button functionality removed - GPIO05 repurposed for fill pump relay control
 * OLED page flip is now controlled via MQTT command from HarmonyOS app
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "oled_display.h"
#include "oled_ssd1306.h"
#include "i2c_common.h"
#include "water_level.h"
#include "ds18b20.h"
#include "tds_sensor.h"
#include "turbidity_sensor.h"
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

// Display refresh interval
#define REFRESH_INTERVAL_MS 200

// I2C initialization delay (wait for I2C_CommonInit to complete)
#define I2C_INIT_DELAY_SEC 2

static int g_current_page = PAGE_SENSORS;
static int g_page_changed = 0;  // Flag to indicate page change request
static int g_oled_initialized = 0;  // Flag to track OLED init status

static void RenderSensorPage(char *line, size_t lineSize)
{
    int waterLevelMM = Get_WaterLevelMM();
    float waterTemp = Get_WaterTemperature();
    int tdsValue = Get_TDSValue();
    int turbidityValue = Get_TurbidityValue();
    int lightIntensity = Get_LightIntensity();
    const TankParams *params = TankControl_GetParams();

    // Line 1: Water Level in mm (YW01 sensor: 0-90mm)
    // Convert params from percentage to mm for display (max 90mm)
    int minMM = (params->waterLevelMin * 90) / 100;
    int maxMM = (params->waterLevelMax * 90) / 100;
    snprintf(line, lineSize, "WL:%dmm(%d-%d)", waterLevelMM, minMM, maxMM);
    OledShowString(0, 1, line, 1);

    // Line 2: Water Temperature
    snprintf(line, lineSize, "WT:%.1fC(%.0f-%.0f)", waterTemp,
             params->waterTempMin, params->waterTempMax);
    OledShowString(0, 2, line, 1);

    // Line 3: TDS
    snprintf(line, lineSize, "TDS:%dppm", tdsValue);
    OledShowString(0, 3, line, 1);

    // Line 4: Turbidity + light intensity (compact format for 128x64 OLED)
    snprintf(line, lineSize, "TUR:%d L:%d", turbidityValue, lightIntensity);
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

    // Line 4: Fan (no percent symbol per user request)
    snprintf(line, lineSize, "Fan: %d", Fan_GetSpeed());
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

    // Wait for I2C to be fully initialized (I2C_CommonInit() in main.c)
    sleep(I2C_INIT_DELAY_SEC);

    // Initialize OLED (I2C0 already initialized in I2C_CommonInit)
    uint32_t ret = OledInit();
    if (ret != 0) {
        printf("[OLED] OledInit failed with error: %u, OLED display disabled\n", ret);
        g_oled_initialized = 0;
        // Continue running but skip OLED operations
        while (1) {
            sleep(1);
        }
    }
    
    g_oled_initialized = 1;
    OledFillScreen(0x00);
    OledShowString(0, 0, "[Sensors]", 1);
    printf("[OLED] Display initialized (manual page flip via MQTT)\n");
    sleep(1);

    // Note: Auto page switching removed
    // Page flip is now controlled via MQTT command from HarmonyOS app

    while (1)
    {
        // Check if page change was requested via OledDisplay_NextPage()
        if (g_page_changed)
        {
            g_page_changed = 0;
            OledFillScreen(0x00);

            const char *titles[] = {"[Sensors]", "[Actuators]", "[System]"};
            OledShowString(0, 0, titles[g_current_page], 1);
            printf("[OLED] Page changed to: %s\n", titles[g_current_page]);
        }

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

        usleep(REFRESH_INTERVAL_MS * 1000);  // Convert ms to us
    }
}

// Debounce for page flip - minimum time between page changes in OS ticks
#define PAGE_CHANGE_DEBOUNCE_TICKS 5  // ~500ms at 100ms per tick

void OledDisplay_NextPage(void)
{
    // Simple time-based debounce using OS tick count
    static uint32_t last_change_tick = 0;
    uint32_t current_tick = osKernelGetTickCount();
    
    // Debounce: ignore if less than debounce ticks since last change
    if (last_change_tick != 0 && (current_tick - last_change_tick) < PAGE_CHANGE_DEBOUNCE_TICKS) {
        printf("[OLED] NextPage ignored - debouncing (delta=%u ticks)\n", 
               (unsigned int)(current_tick - last_change_tick));
        return;
    }
    
    last_change_tick = current_tick;
    g_current_page = (g_current_page + 1) % PAGE_COUNT;
    g_page_changed = 1;
    printf("[OLED] NextPage requested, switching to page %d\n", g_current_page);
}

int OledDisplay_GetCurrentPage(void)
{
    return g_current_page;
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

/**
 * @file tank_control.c
 * @brief Main tank control logic implementation for aquatic plant tank
 * Coordinates all sensors and actuators based on plant parameters
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "tank_control.h"
#include "water_level.h"
#include "ds18b20.h"
#include "tds_sensor.h"
#include "light_sensor.h"
#include "pump_control.h"
#include "temp_control.h"
#include "led_control.h"
#include "alarm.h"

#define TANK_CONTROL_TASK_STACK_SIZE 4096

/**
 * Default parameters for different plant types
 * Format: {waterLevelMin(%), waterLevelMax(%), waterTempMin(C), waterTempMax(C),
 *          lightThreshold(%), lightDuration(hours), tdsMin(ppm), tdsMax(ppm)}
 * Index: 0-Pothos, 1-Water Lily, 2-Pennywort, 3-Water Hyacinth,
 *        4-Lucky Bamboo, 5-Bowl Lotus, 6-Narcissus, 7-Hyacinth
 */
static const TankParams g_plant_params[] = {
    // 绿萝 (Pothos)
    {20, 60, 18.0f, 28.0f, 30, 8, 100, 500},
    // 睡莲 (Water Lily)
    {50, 90, 22.0f, 30.0f, 80, 10, 100, 500},
    // 铜钱草 (Pennywort)
    {30, 70, 16.0f, 26.0f, 40, 6, 100, 500},
    // 水葫芦 (Water Hyacinth)
    {40, 80, 20.0f, 32.0f, 60, 8, 100, 500},
    // 富贵竹 (Lucky Bamboo)
    {25, 50, 18.0f, 25.0f, 20, 6, 100, 500},
    // 碗莲 (Bowl Lotus)
    {45, 85, 20.0f, 28.0f, 70, 10, 100, 500},
    // 水仙 (Narcissus)
    {20, 40, 10.0f, 18.0f, 50, 8, 100, 500},
    // 风信子 (Hyacinth)
    {15, 35, 12.0f, 20.0f, 40, 6, 100, 500}
};

static TankParams g_current_params = {
    .waterLevelMin = 30,
    .waterLevelMax = 80,
    .waterTempMin = 20.0f,
    .waterTempMax = 28.0f,
    .lightThreshold = 30,
    .lightDuration = 8,
    .tdsMin = 100,
    .tdsMax = 500
};

static ControlMode g_control_mode = CONTROL_MODE_AUTO;

// Safety thresholds for alarms
#define WATER_LEVEL_CRITICAL_LOW 10
#define WATER_LEVEL_CRITICAL_HIGH 95
#define WATER_TEMP_CRITICAL_LOW 5.0f
#define WATER_TEMP_CRITICAL_HIGH 35.0f
#define TDS_CRITICAL_HIGH 800

void TankControl_Init(void)
{
    // Initialize all actuators
    Pump_Init();
    TempControl_Init();
    LED_Init();
    Alarm_Init();

    printf("[TankControl] Initialized with default parameters\n");
    printf("[TankControl] WaterLevel: %d-%d%%, Temp: %.1f-%.1fC, Light: %d%%\n",
           g_current_params.waterLevelMin, g_current_params.waterLevelMax,
           g_current_params.waterTempMin, g_current_params.waterTempMax,
           g_current_params.lightThreshold);
}

void TankControl_SetMode(ControlMode mode)
{
    g_control_mode = mode;
    printf("[TankControl] Mode set to: %s\n", mode == CONTROL_MODE_AUTO ? "AUTO" : "MANUAL");

    // Stop all actuators when switching to manual mode
    if (mode == CONTROL_MODE_MANUAL)
    {
        Pump_StopAll();
        Fan_Stop();
        Heater_Off();
    }
}

ControlMode TankControl_GetMode(void)
{
    return g_control_mode;
}

void TankControl_SetParams(const TankParams *params)
{
    if (params != NULL)
    {
        memcpy(&g_current_params, params, sizeof(TankParams));
        printf("[TankControl] Parameters updated\n");
        printf("[TankControl] WaterLevel: %d-%d%%, Temp: %.1f-%.1fC, Light: %d%%\n",
               g_current_params.waterLevelMin, g_current_params.waterLevelMax,
               g_current_params.waterTempMin, g_current_params.waterTempMax,
               g_current_params.lightThreshold);
    }
}

const TankParams* TankControl_GetParams(void)
{
    return &g_current_params;
}

void TankControl_SetPlantType(int plantType)
{
    if (plantType >= 0 && plantType < 8)
    {
        memcpy(&g_current_params, &g_plant_params[plantType], sizeof(TankParams));
        printf("[TankControl] Plant type set to %d\n", plantType);
    }
}

// Manual control functions
void TankControl_ManualLED(int on)
{
    if (g_control_mode == CONTROL_MODE_MANUAL)
    {
        if (on) LED_On();
        else LED_Off();
    }
}

void TankControl_ManualFillPump(int on)
{
    if (g_control_mode == CONTROL_MODE_MANUAL)
    {
        Pump_SetState(PUMP_FILL, on ? PUMP_ON : PUMP_OFF);
    }
}

void TankControl_ManualDrainPump(int on)
{
    if (g_control_mode == CONTROL_MODE_MANUAL)
    {
        Pump_SetState(PUMP_DRAIN, on ? PUMP_ON : PUMP_OFF);
    }
}

void TankControl_ManualHeater(int on)
{
    if (g_control_mode == CONTROL_MODE_MANUAL)
    {
        if (on) Heater_On();
        else Heater_Off();
    }
}

void TankControl_ManualFan(int speed)
{
    if (g_control_mode == CONTROL_MODE_MANUAL)
    {
        Fan_SetSpeed(speed);
    }
}

// Check for safety conditions and trigger alarms
static void CheckSafetyConditions(int waterLevel, float waterTemp, int tdsValue)
{
    char alarmMsg[64] = "";

    // Critical water level check
    if (waterLevel <= WATER_LEVEL_CRITICAL_LOW)
    {
        snprintf(alarmMsg, sizeof(alarmMsg), "Water level critical: %d%%", waterLevel);
        Alarm_Trigger(ALARM_DANGER, alarmMsg);
        Pump_StopDrain();  // Prevent dry running
        Heater_Off();      // Prevent dry burning
        return;
    }

    if (waterLevel >= WATER_LEVEL_CRITICAL_HIGH)
    {
        snprintf(alarmMsg, sizeof(alarmMsg), "Water level too high: %d%%", waterLevel);
        Alarm_Trigger(ALARM_WARNING, alarmMsg);
        Pump_StopFill();
        return;
    }

    // Critical temperature check
    if (waterTemp >= WATER_TEMP_CRITICAL_HIGH)
    {
        snprintf(alarmMsg, sizeof(alarmMsg), "Water temp critical: %.1fC", waterTemp);
        Alarm_Trigger(ALARM_DANGER, alarmMsg);
        Heater_Off();
        Fan_SetSpeed(100);  // Maximum cooling
        return;
    }

    if (waterTemp <= WATER_TEMP_CRITICAL_LOW)
    {
        snprintf(alarmMsg, sizeof(alarmMsg), "Water temp too low: %.1fC", waterTemp);
        Alarm_Trigger(ALARM_WARNING, alarmMsg);
        return;
    }

    // TDS (water quality) check
    if (tdsValue >= TDS_CRITICAL_HIGH)
    {
        snprintf(alarmMsg, sizeof(alarmMsg), "Water quality poor: %dppm", tdsValue);
        Alarm_Trigger(ALARM_WARNING, alarmMsg);
        return;
    }

    // All conditions normal
    if (Alarm_GetLevel() != ALARM_NONE)
    {
        Alarm_Stop();
    }
}

// Automatic water level control
static void AutoWaterLevelControl(int waterLevel)
{
    if (waterLevel < g_current_params.waterLevelMin)
    {
        // Water level too low - start filling
        if (Pump_GetState(PUMP_FILL) == PUMP_OFF)
        {
            Pump_StartFill();
            printf("[TankControl] Auto: Starting fill pump (level: %d%% < %d%%)\n",
                   waterLevel, g_current_params.waterLevelMin);
        }
    }
    else if (waterLevel >= g_current_params.waterLevelMax)
    {
        // Water level reached upper limit - stop filling
        if (Pump_GetState(PUMP_FILL) == PUMP_ON)
        {
            Pump_StopFill();
            printf("[TankControl] Auto: Stopping fill pump (level: %d%% >= %d%%)\n",
                   waterLevel, g_current_params.waterLevelMax);
        }
    }
}

// Automatic temperature control
static void AutoTemperatureControl(float waterTemp)
{
    if (waterTemp < g_current_params.waterTempMin)
    {
        // Temperature too low - enable heater, stop fan
        if (!Heater_GetState())
        {
            Heater_On();
            printf("[TankControl] Auto: Heater ON (temp: %.1fC < %.1fC)\n",
                   waterTemp, g_current_params.waterTempMin);
        }
        if (Fan_GetSpeed() > 0)
        {
            Fan_Stop();
        }
    }
    else if (waterTemp > g_current_params.waterTempMax)
    {
        // Temperature too high - enable fan, stop heater
        if (Heater_GetState())
        {
            Heater_Off();
            printf("[TankControl] Auto: Heater OFF (temp: %.1fC > %.1fC)\n",
                   waterTemp, g_current_params.waterTempMax);
        }
        // Calculate fan speed based on temperature difference
        float tempDiff = waterTemp - g_current_params.waterTempMax;
        int fanSpeed = (int)(tempDiff * 20);  // 20% per degree over max
        if (fanSpeed < 30) fanSpeed = 30;
        if (fanSpeed > 100) fanSpeed = 100;
        if (Fan_GetSpeed() != fanSpeed)
        {
            Fan_SetSpeed(fanSpeed);
            printf("[TankControl] Auto: Fan at %d%% (temp: %.1fC)\n", fanSpeed, waterTemp);
        }
    }
    else
    {
        // Temperature in optimal range - turn off both
        if (Heater_GetState())
        {
            Heater_Off();
        }
        if (Fan_GetSpeed() > 0)
        {
            Fan_Stop();
        }
    }
}

// Automatic light control
static void AutoLightControl(int lightIntensity)
{
    if (lightIntensity < g_current_params.lightThreshold)
    {
        // Too dark - enable LED
        if (!LED_GetState())
        {
            LED_On();
            printf("[TankControl] Auto: LED ON (light: %d%% < %d%%)\n",
                   lightIntensity, g_current_params.lightThreshold);
        }
    }
    else
    {
        // Sufficient light - disable LED
        if (LED_GetState())
        {
            LED_Off();
            printf("[TankControl] Auto: LED OFF (light: %d%% >= %d%%)\n",
                   lightIntensity, g_current_params.lightThreshold);
        }
    }
}

static void TankControl_Task(void *arg)
{
    (void)arg;

    printf("[TankControl] Control task started\n");

    while (1)
    {
        // Read all sensor values
        int waterLevel = Get_WaterLevelPercent();
        float waterTemp = Get_WaterTemperature();
        int tdsValue = Get_TDSValue();
        int lightIntensity = Get_LightIntensity();

        // Always check safety conditions regardless of mode
        CheckSafetyConditions(waterLevel, waterTemp, tdsValue);

        // Apply automatic control if in AUTO mode
        if (g_control_mode == CONTROL_MODE_AUTO)
        {
            AutoWaterLevelControl(waterLevel);
            AutoTemperatureControl(waterTemp);
            AutoLightControl(lightIntensity);
        }

        // Control loop runs every 2 seconds
        sleep(2);
    }
}

void TankControl_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "TankControl_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TANK_CONTROL_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;  // Higher priority for control

    if (osThreadNew((osThreadFunc_t)TankControl_Task, NULL, &attr) == NULL)
    {
        printf("[TankControl] Failed to create task!\n");
    }
}

/**
 * @file alarm.c
 * @brief Alarm (beeper) control implementation for aquatic plant tank
 * Uses GPIO09 for active beeper control (LOW = ON, HIGH = OFF)
 * 
 * Note: This is for an ACTIVE buzzer with low-level trigger.
 * Active buzzers have built-in oscillators and only need DC voltage.
 * LOW level triggers the buzzer, HIGH level stops it.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "alarm.h"

// Beeper control pin (GPIO09)
#define BEEPER_GPIO WIFI_IOT_GPIO_IDX_9
#define BEEPER_IO   WIFI_IOT_IO_NAME_GPIO_9

static AlarmLevel g_alarm_level = ALARM_NONE;
static char g_alarm_message[64] = "";

void Alarm_Init(void)
{
    // Configure GPIO09 as output for active buzzer control
    IoSetFunc(BEEPER_IO, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(BEEPER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    // Start with buzzer OFF (HIGH = OFF for active-low buzzer)
    GpioSetOutputVal(BEEPER_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    g_alarm_level = ALARM_NONE;
    g_alarm_message[0] = '\0';

    printf("[Alarm] Initialized (active-low buzzer)\n");
}

// Turn buzzer ON (LOW level for active-low buzzer)
static void Beeper_On(void)
{
    GpioSetOutputVal(BEEPER_GPIO, WIFI_IOT_GPIO_VALUE0);
}

// Turn buzzer OFF (HIGH level for active-low buzzer)
static void Beeper_Off(void)
{
    GpioSetOutputVal(BEEPER_GPIO, WIFI_IOT_GPIO_VALUE1);
}

void Alarm_Trigger(AlarmLevel level, const char *message)
{
    g_alarm_level = level;
    if (message != NULL)
    {
        strncpy(g_alarm_message, message, sizeof(g_alarm_message) - 1);
        g_alarm_message[sizeof(g_alarm_message) - 1] = '\0';
    }
    else
    {
        g_alarm_message[0] = '\0';
    }

    if (level == ALARM_WARNING)
    {
        // Short beep for warning
        Beeper_On();
        usleep(500000);  // 500ms
        Beeper_Off();
    }
    else if (level == ALARM_DANGER)
    {
        // Continuous beeping for danger
        for (int i = 0; i < 5; i++)
        {
            Beeper_On();
            usleep(200000);  // 200ms
            Beeper_Off();
            usleep(100000);  // 100ms pause
        }
    }
}

void Alarm_Stop(void)
{
    Beeper_Off();
    g_alarm_level = ALARM_NONE;
    g_alarm_message[0] = '\0';
}

AlarmLevel Alarm_GetLevel(void)
{
    return g_alarm_level;
}

const char* Alarm_GetMessage(void)
{
    return g_alarm_message;
}

void Alarm_Beep(void)
{
    Beeper_On();
    usleep(100000);  // 100ms beep
    Beeper_Off();
}

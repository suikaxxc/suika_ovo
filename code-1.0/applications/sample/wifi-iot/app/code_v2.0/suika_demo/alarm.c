/**
 * @file alarm.c
 * @brief Alarm (beeper) control implementation for aquatic plant tank
 * Uses PWM0 (GPIO09) for beeper control
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"

#include "alarm.h"

// Beeper control pin (PWM0/GPIO09)
#define BEEPER_GPIO WIFI_IOT_GPIO_IDX_9
#define BEEPER_IO   WIFI_IOT_IO_NAME_GPIO_9
#define BEEPER_PWM_PORT WIFI_IOT_PWM_PORT_PWM0

// Beeper frequency divisor for ~2700Hz (typical beeper resonance)
#define BEEPER_FREQ_DIVISOR 34052

static AlarmLevel g_alarm_level = ALARM_NONE;
static char g_alarm_message[64] = "";

void Alarm_Init(void)
{
    GpioInit();

    IoSetFunc(BEEPER_IO, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(BEEPER_PWM_PORT);
    g_alarm_level = ALARM_NONE;
    g_alarm_message[0] = '\0';

    printf("[Alarm] Initialized\n");
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

    printf("[Alarm] Triggered: level=%d, msg=%s\n", level, g_alarm_message);

    if (level == ALARM_WARNING)
    {
        // Short beep for warning
        PwmStart(BEEPER_PWM_PORT, BEEPER_FREQ_DIVISOR / 2, BEEPER_FREQ_DIVISOR);
        usleep(500000);  // 500ms
        PwmStop(BEEPER_PWM_PORT);
    }
    else if (level == ALARM_DANGER)
    {
        // Continuous beeping for danger
        for (int i = 0; i < 5; i++)
        {
            PwmStart(BEEPER_PWM_PORT, BEEPER_FREQ_DIVISOR / 2, BEEPER_FREQ_DIVISOR);
            usleep(200000);  // 200ms
            PwmStop(BEEPER_PWM_PORT);
            usleep(100000);  // 100ms pause
        }
    }
}

void Alarm_Stop(void)
{
    PwmStop(BEEPER_PWM_PORT);
    g_alarm_level = ALARM_NONE;
    g_alarm_message[0] = '\0';
    printf("[Alarm] Stopped\n");
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
    PwmStart(BEEPER_PWM_PORT, BEEPER_FREQ_DIVISOR / 2, BEEPER_FREQ_DIVISOR);
    usleep(100000);  // 100ms beep
    PwmStop(BEEPER_PWM_PORT);
}

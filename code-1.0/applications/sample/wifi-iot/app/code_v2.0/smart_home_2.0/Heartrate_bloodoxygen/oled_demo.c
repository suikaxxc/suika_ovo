#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "oled_ssd1306.h"

#define OLED_TASK_STACK_SIZE 4096

void init(void)
{
    GpioInit();
    OledInit();
}

static void Oled_Task(void *arg)
{
    (void)arg;
    init();
    OledFillScreen(0x00); // 清屏
    OledShowString(0, 0, "HarmonyOS", 1);
    sleep(1);
}

static void Oled_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Oled_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = OLED_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew(Oled_Task, NULL, &attr) == NULL)
    {
        printf("[OledDemo] Falied to create OledTask!\n");
    }
}

APP_FEATURE_INIT(Oled_MainLoop);

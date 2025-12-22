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

#include "aht20.h"
#include "i2c_common.h"
#include "oled_ssd1306.h"
#include "beeper_demo.h"
#include "oled_demo.h"
#include "aht20_demo.h"
#include "mq2_demo.h"
#include "ldr_demo.h"
#include "ldr_led_control.h"
#include "yw01_demo.h"
#include "publish_task.h"

#define TASK_STACK_SIZE 4096

// 综合业务函数
static void Environment_Task(void *arg)
{
    (void)arg;
    Mq2_MainLoop();   // 可燃气体数据采集
    AHT20_MainLoop(); // 温湿度数据采集
    Ldr_MainLoop();   // 光敏传感器数据采集
    LdrLed_MainLoop(); // 光照联动LED控制（占用 GPIO9）
    YW01_MainLoop();  // 液位传感器数据采集
    Beep_Task();   // 启用蜂鸣器数据报警（使用 PWM0/GPIO9，与LED复用，避免同时用）
    Oled_MainLoop();  // oled的数据显示
    publish_task();   // 启动MQTT发布任务
}

static void MainTask(void)
{
    osThreadAttr_t attr;
    attr.name = "Environment_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    // 统一初始化 I2C0 与互斥锁，防止 OLED 与 AHT20 并发冲突
    I2C_CommonInit();

    if (osThreadNew((osThreadFunc_t)Environment_Task, NULL, &attr) == NULL)
    {
        printf("[EnvironmentDemo] Falied to create EnvironmentTask!\n");
    }
}

APP_FEATURE_INIT(MainTask);
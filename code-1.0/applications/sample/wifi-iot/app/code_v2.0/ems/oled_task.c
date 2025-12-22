#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "oled_ssd1306.h"
#include "enrionment_task.h"

// 对GPIO管脚及OLED进行初始化
void oled_task_init(void)
{
    GpioInit();
    OledInit();
}

// 任务函数，用于在OLED上显示环境数据
void oled_task_run(void *arg)
{
    (void)arg;
    oled_task_init();     // 初始化
    OledFillScreen(0x00); // 清屏，
    // 在左上角位置显示字符串Hello, HarmonyOS
    OledShowString(0, 0, "hello,chinasoft", 1);
    sleep(1); // 等待1秒
    char line[32] = {0};
    OledFillScreen(0x00); // 清屏
    while (1)
    {
        OledShowString(16, 0, "Environment", 1);

        // 组装显示温度的字符串
        snprintf(line, sizeof(line), "temp: %.1f deg", temperature);
        OledShowString(0, 3, line, 1); // 在（0，1）位置显示组装后的温度字符串

        // 组装显示湿度的字符串
        snprintf(line, sizeof(line), "humi: %.1f %%", humidity);
        OledShowString(0, 5, line, 1); // 在（0，2）位置显示组装后的湿度字符串

        // 组装显示气体的字符串 单位是百万分之  ，检测浓度：300-10000ppm(可燃气体)
        snprintf(line, sizeof(line), "gas : %4d ppm", gas);
        OledShowString(0, 7, line, 1); // 在（0，3）位置显示组装后的气体字符串
        usleep(500000);
        ; // 睡眠
    }
}

// 模块函数oled_task
void oled_task(void)
{
    osThreadAttr_t attr;
    attr.name = "oled_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(oled_task_run, NULL, &attr) == NULL)
    {
        printf("[oled_task] Falied to create oled_task!\n");
    }
}

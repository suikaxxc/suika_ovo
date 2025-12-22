#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"
#include "oled_ssd1306.h"

#include "aht20_demo.h"
#include "mq2_demo.h"
#include "ldr_demo.h"
#include "ldr_led_control.h"
#include "yw01_demo.h"

#define OLED_TASK_STACK_SIZE 4096
#define OLED_PAGE_SENSOR 0
#define OLED_PAGE_ACTUATOR 1

/* 
   按键修改为 ADC 方式读取 (GPIO5 / ADC2)
   S1: 按下时电压较低 (模拟值 < 100)
   S2: 按下时电压居中 (模拟值 400 ~ 1200)
   无按键: 高电平 (模拟值 > 3000)
*/
#define OLED_BTN_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_2

void OLED_init(void)
{
    GpioInit();
    OledInit();
}

static void ButtonInit(void)
{
    /* 
       ADC 模式下通常不需要配置 GPIO 方向，
       但如果依赖内部上拉电阻来分压，则需要保持上拉配置。
       此处保留上拉配置以防万一。
    */
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_5, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);
}

static void RenderSensorPage(char *line, size_t lineSize)
{
    // 显示温度值
    snprintf(line, lineSize, "temp: %.2f", Get_Temperature());
    OledShowString(0, 1, line, 1);

    // 显示湿度值
    snprintf(line, lineSize, "humi: %.2f", Get_Humidity());
    OledShowString(0, 2, line, 1);

    // 显示气体值
    snprintf(line, lineSize, "gas: %d", Get_Mq2Value());
    OledShowString(0, 3, line, 1);

    // 显示光照强度（去掉百分号）
    snprintf(line, lineSize, "light: %d", Get_LightPercent());
    OledShowString(0, 4, line, 1);

    // 显示液位数值（mm）
    snprintf(line, lineSize, "level: %dmm", Get_YW01Mm());
    OledShowString(0, 5, line, 1);
}

static void RenderActuatorPage(char *line, size_t lineSize)
{
    OledShowString(0, 1, "Actuators", 1);

    snprintf(line, lineSize, "LED(LDR): %s", LdrLed_GetState() ? "ON " : "OFF");
    OledShowString(0, 2, line, 1);

    OledShowString(0, 3, "Beeper: --", 1);
    OledShowString(0, 4, "Relay:  --", 1);
    OledShowString(0, 5, "Reserved", 1);
}

static void Oled_Task(void *arg)
{
    (void)arg;
    static char line[32] = {0};
    int page = OLED_PAGE_SENSOR;
    int lastBtnState = 0; // 0: None, 1: S1, 2: S2

    OLED_init();
    ButtonInit();
    OledFillScreen(0x00); // 清屏
    OledShowString(0, 0, "Hello, HarmonyOS", 1);
    sleep(1);
    while (1)
    {
        unsigned short adc_val = 0;
        int currentBtnState = 0;

        /* 读取 ADC 值区分按键 */
        if (AdcRead(OLED_BTN_ADC_CHANNEL, &adc_val, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            if (adc_val < 200) {
                currentBtnState = 1; // S1 Pressed
            } else if (adc_val > 400 && adc_val < 1200) {
                currentBtnState = 2; // S2 Pressed
            } else {
                currentBtnState = 0; // None
            }
        }

        /* 按键按下处理 (上升沿) */
        if (lastBtnState == 0 && currentBtnState != 0)
        {
            if (currentBtnState == 1) // S1: 切换页面
            {
                printf("[OLED] S1 Pressed -> Switch Page\n");
                page = (page == OLED_PAGE_SENSOR) ? OLED_PAGE_ACTUATOR : OLED_PAGE_SENSOR;
                OledFillScreen(0x00);
                OledShowString(0, 0, (page == OLED_PAGE_SENSOR) ? "Sensors" : "Status", 1);
            }
            else if (currentBtnState == 2) // S2: 仅打印日志 (可扩展功能)
            {
                printf("[OLED] S2 Pressed\n");
            }
        }
        lastBtnState = currentBtnState;

        if (page == OLED_PAGE_SENSOR)
        {
            RenderSensorPage(line, sizeof(line));
        }
        else
        {
            RenderActuatorPage(line, sizeof(line));
        }

        usleep(200000);
    }
}

void Oled_MainLoop(void)
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

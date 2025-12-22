#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"
#include "aht20.h"
#include "wifiiot_adc.h"

// 400KHz
#define AHT20_BAUDRATE 400 * 1000

// I2C0
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

// ADC5
#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

// 对部变量进行初始化
float humidity = 0.0f;    // 湿度，单位为：%
float temperature = 0.0f; // 温度，单位为：摄氏度
unsigned short gas = 0;   // 单位为：ppm，ppm浓度（parts per million）是用溶质质量占全部溶液质量的百万分比来表示的浓度
                          //  MQ-2测试范围为：300-10000ppm
unsigned short stat = 0;  // 0正常，1，警告;2，危险

// 初始化 GPIO和I2C
void init(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
}

// 任务函数
static void enrionment_task_run(void *arg)
{
    (void)arg;
    init(); // 初始化IIC
    uint32_t retval = 0;

    // 发送初始化校准命令
    while (WIFI_IOT_SUCCESS != AHT20_Calibrate())
    {
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }
    while (1)
    {
        // 发送 触发测量 命令，开始测量
        retval = AHT20_StartMeasure();
        if (retval != WIFI_IOT_SUCCESS)
        {
            printf("trigger measure failed!\r\n");
        }
        else
        {
            // 接收测量结果，拼接转换为标准值
            retval = AHT20_GetMeasureResult(&temperature, &humidity);
        }
        // 读取可燃气体值  检测浓度：300-10000ppm(可燃气体)
        AdcRead(GAS_SENSOR_CHAN_NAME, &gas, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        if (temperature < 10 || temperature > 50 || humidity > 80 || gas > 1840)
        {
            stat = 2; // 危险
        }
        else if (temperature < 20 || temperature > 40 || humidity > 70 || gas > 1760)
        {
            stat = 1; // 警告
        }
        else
        {
            stat = 0; // 正常
        }
        printf("temp:%4.1f degree,hum:%4.1f %%,gas:%d PPM,stat:%d  \r\n", temperature, humidity, gas, stat);
        //    usleep(500000);  //微秒
        sleep(1); // 1秒
    }
}

void enrionment_task(void)
{
    osThreadAttr_t attr;

    attr.name = "enrionment_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; // 线程栈的大小
    attr.priority = osPriorityNormal;

    if (osThreadNew(enrionment_task_run, NULL, &attr) == NULL)
    {
        printf("[enrionment_thread] Falied to create enrionment_task!\n");
    }
}
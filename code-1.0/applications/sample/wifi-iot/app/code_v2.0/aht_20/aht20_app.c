#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"
#include "aht20.h"

#define AHT20_BAUDRATE 400 * 1000
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

void init(void)
{
    // 初始化GPIO
    GpioInit();

    // 设置GPIO的复用功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    // 初始化I2c
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
}

// 业务函数
static void aht20_task_run(void *arg)
{
    (void)arg;
    init(); // 初始化IIC
    uint32_t retval = 0;
    float humidity = 0.0f;    // 用于保存湿度的变量
    float temperature = 0.0f; // 用于保存温度的变量

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
            printf("temp: %.2f,  humi: %.2f\n", temperature, humidity);
        }
        sleep(1);
    }
}

// 创建新任务来运行业务函数 aht20_task_run
static void aht20_task(void)
{
    osThreadAttr_t attr;

    attr.name = "Aht20Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(aht20_task_run, NULL, &attr) == NULL)
    {
        printf("[Aht20Demo] Falied to create Aht20Task!\n");
    }
}

SYS_RUN(aht20_task);
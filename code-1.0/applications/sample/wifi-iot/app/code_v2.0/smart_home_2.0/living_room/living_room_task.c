#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"
#include "wifiiot_i2c.h"

//对应管脚PIN11
#include "aht20.h"

#define AHT20_BAUDRATE 400 * 1000
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

int fire_alarm = 0;            //获取火焰状态,0表示探测到火焰，1表示未探测到
unsigned short fire_state = 0; // 1，表示发生火灾，0表示未发生火灾
float humidity = 0.0f;         //用于保存湿度的变量
float temperature = 0.0f;      //用于保存温度的变量

void init(void)
{
    GpioInit();

    // 蜂鸣器引脚 设置为 PWM功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE); //初始化I2c 的波特率

    //火焰接收器引脚
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_GPIO_DIR_IN);
}

void living_room_entry(void *arg)
{
    (void)arg;
    init(); //初始化IIC
    uint32_t retval = 0;
    uint16_t freqDivisor = 34052;

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
           // printf("temp: %.2f,  humi: %.2f\n", temperature, humidity);
        }

        //火焰探测
        WifiIotGpioValue value;
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_5, &value); //获取火焰状态,0表示探测到火焰，1表示未探测到

        //获取到的值转化为火灾状态
        fire_state = value ? 0 : 1;

        if (fire_state)
        {
            fire_alarm = 1;
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor / 20, freqDivisor);
        }
        else
        {
            fire_alarm = 0;
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }

        sleep(1);
    }
}

void living_room_task(void)
{

    osThreadAttr_t attr;
    attr.name = "living_room_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(living_room_entry, NULL, &attr) == NULL)
    {
        printf("[living_room_task] Falied to create living_room_task!\n");
    }
}

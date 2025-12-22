#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"


//对应管脚PIN11
#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

//气体报警阈值
#define GAS_THRESHOLD 1000

int alarmbell = 0;
int gas = 0;////获取读取到的燃气值

int lastGasData = 0;

void init(void)
{
    GpioInit();

    // 蜂鸣器引脚 设置为 PWM功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);
}
//
void kitchen_entry(void *arg)
{
    (void)arg;

    init();
    while (1)
    {

        unsigned short data = 0; //保存读取到的燃气值
        //调用AdcRead读取值
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            gas = data;

            if (gas > GAS_THRESHOLD)
            {
                alarmbell = 1;
                uint16_t freqDivisor = 34052;
                //                                  占空比，        频率
                PwmStart(WIFI_IOT_PWM_PORT_PWM0, gas * gas * 3 / GAS_THRESHOLD, freqDivisor);
            }
            else
            {
                alarmbell = 0;
                PwmStop(WIFI_IOT_PWM_PORT_PWM0);
            }
            sleep(1);
        }
    }
} 

/* void kitchen_entry(void *arg)
{
    (void)arg;

    init();

    while (1)
    {

        unsigned short data = 0; //保存读取到的燃气值
        //调用AdcRead读取值
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {

            //lastGasData 跟 data做判断，如果上次值跟本次值差值在20之内，终止本次循环，去取下次循环的data值
            gas = data;
           // int difference = abs(gas - lastGasData);
            lastGasData = gas;
            // if(difference < 20){
            //     continue;
            // }
            
            if (gas > GAS_THRESHOLD)
            {
                alarmbell = 1;
                uint16_t freqDivisor = 34052;
                //                                  占空比，        频率
                PwmStart(WIFI_IOT_PWM_PORT_PWM0, gas * gas * 3 / GAS_THRESHOLD, freqDivisor);
            }
            else
            {
                alarmbell = 0;
                PwmStop(WIFI_IOT_PWM_PORT_PWM0);
            }
            sleep(1);
        }
    }
} */

void kitchen_task(void)
{

    osThreadAttr_t attr;
    attr.name = "kitchen_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(kitchen_entry, NULL, &attr) == NULL)
    {
        printf("[kitchen_entry] Falied to create kitchen_entry!\n");
    }
}

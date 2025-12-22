#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "hi_pwm.h"
#include "enrionment_task.h"

void beepInit(void)
{
    GpioInit();

    // 蜂鸣器引脚 设置为 PWM功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);
}
/**
 *     38223, // 1046.5
    34052, // 1174.7
    30338, // 1318.5
    28635, // 1396.9
    25511, // 1568
    22728, // 1760
    20249, // 1975.5
    51021  // 5_ 783.99 // 第一个八度的 5
    */

static void beeperThread(void *arg){
    (void)arg;
    uint16_t freqDivisor = 34052;
    beepInit();
    freqDivisor=34052;
    while (1)
    {

        if(stat==0){//安全级别 停止所有PWM方波输出
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }else if(stat ==1){
            //警告级别，占空比较为freqDivisor / 10
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor /30, freqDivisor);

        }else{
            //危险级别，占空比较为freqDivisor / 2
             PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor/3 , freqDivisor);
        }
        
        
      sleep(1);
    }
}

 void beeperTask(void){
    osThreadAttr_t attr;
    
    attr.name = "beeperTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew(beeperThread, NULL, &attr) == NULL)
    {
        printf("[beeperTask] Falied to create enrionmentThread!\n");
    }
}

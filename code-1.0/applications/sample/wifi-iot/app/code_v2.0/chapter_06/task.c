#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "stdio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

//线程执行回调函数
static void *example_task_entry(void *data)
{
    data = data;
    while(1)
    {
        printf("\r\n\r\nExample task is running!\r\n\r\n");
        usleep(1000000);
    }

    return data;
}

void example_task_init(void)
{
    osThreadAttr_t attr;

    attr.name = "example_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;
    //创建新任务
    if (osThreadNew((osThreadFunc_t)example_task_entry, NULL, &attr) == NULL)    {
        printf("Example_task create failed!\n");
    }

    while(1)
    {
        printf("\r\n\r main is running!\r\n\r\n");
        usleep(4000000);
    }

}


APP_FEATURE_INIT(example_task_init);



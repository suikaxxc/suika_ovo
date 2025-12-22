#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "kitchen_task.h"
#include "publish_task.h"
#include "oled_task.h"

static void entry(void *arg)
{
    (void)arg;
    sleep(1);//等待1秒
    printf("Smart Home 2.0 Running\n");
    kitchen_task();//厨房可燃气体监测报警模块
    oled_task();//数据在OLED上显示
    publish_task();//数据发布
}

void sh_task(void)
{
    osThreadAttr_t attr;

    attr.name = "smart_home_2.0";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)entry, NULL, &attr) == NULL)
    {
        printf("[smart_home_2.0] Falied to create sh_task!\n");
    }
}

APP_FEATURE_INIT(sh_task);
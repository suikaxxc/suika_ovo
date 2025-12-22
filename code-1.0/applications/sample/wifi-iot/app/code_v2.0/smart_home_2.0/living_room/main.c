#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "living_room_task.h"
#include "publish_task.h"
#include "oled_task.h"

static void entry(void *arg)
{
    (void)arg;
    sleep(1);
    printf("Smart Home 2.0 Running\n");
    living_room_task();
    oled_task();
    publish_task();
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
        printf("[smart_home_2.0] Falied to create ss_task!\n");
    }
}

SYS_RUN(sh_task);
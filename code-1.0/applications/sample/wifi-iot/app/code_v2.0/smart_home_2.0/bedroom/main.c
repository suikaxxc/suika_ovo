#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "bedroom_task.h"
#include "mqtt_task.h"

static void sh_thread(void *arg)
{
    (void)arg;

    sleep(1);
    printf("Smart Home 2.0 Running\n");

    bedroom_task();
    mqtt_task();
}

void sh_entry(void)
{
    osThreadAttr_t attr;

    attr.name = "smart_home_2.0";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)sh_thread, NULL, &attr) == NULL)
    {
        printf("[smart_home_2.0] Falied to create sh_thread!\n");
    }
}

SYS_RUN(sh_entry);
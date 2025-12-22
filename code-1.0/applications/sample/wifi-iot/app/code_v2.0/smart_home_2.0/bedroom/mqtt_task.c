#include "cmsis_os2.h"
#include <unistd.h>
#include <stdio.h>
#include "wifi_utils.h"
#include "mqtt_utils.h"

static void mqtt_thread(void *arg)
{
    (void)arg;
    connect_wifi(); //连接wifi
    sleep(1);       //等待wifi
    mqtt_connect(); //调用mqtt实现实时数据发布
}

void mqtt_task(void)
{
    osThreadAttr_t attr;

    attr.name = "mqtt_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // 4096;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)mqtt_thread, NULL, &attr) == NULL)
    {
        printf("[mqtt_task] Falied to create mqtt_thread!\n");
    }
}

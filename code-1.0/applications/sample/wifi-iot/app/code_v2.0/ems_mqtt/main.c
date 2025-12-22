#include <stdio.h>
#include "ohos_init.h"
#include <unistd.h>
#include "cmsis_os2.h"
#include "enrionment_task.h"
#include "oled_task.h"
#include "publish_task.h"

// 主模块
static void ems_thread(void *arg)
{
    (void)arg;
    sleep(2);
    printf("Environmental monitoring system running\n");

    enrionment_task(); // 调用 环境监测子模块
    oled_task();       // 添加 显示模块
    publish_task();    // 添加 发布模块
}

// 创建线程运行主模块
void ems_entry(void)
{
    osThreadAttr_t attr;

    attr.name = "ems_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // 4096;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)ems_thread, NULL, &attr) == NULL)
    {
        printf("[ems_main] Falied to create ems_main!\n");
    }
}

SYS_RUN(ems_entry); // 初始化主模块
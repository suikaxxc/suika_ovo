#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
#include "wifi_utils.h"
#include "mqtt_test.h"
#include <at.h>
#include <hi_at.h>
#include "traffic_light.h"
#include "oled_ssd1306.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "oled_ssd1306.h"
#include "beeper_music.h"

static void smart_home_platform_thread(void *arg)
{
    (void)arg;
    printf("smart_home_platform running\n");
    connect_wifi();
    initLights();
    StartBeepMusicTask();//启动播放音乐线程
    OledInit();//
    mqtt_connect();
}

hi_u32 at_exe_smart_home_platform_cmd(void)
{
    osThreadAttr_t attr;

    attr.name = "smart_home_platform_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8192 * 5; // 4096;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)smart_home_platform_thread, NULL, &attr) == NULL)
    {
        printf("[smart_home_platform] Falied to create LedTask!\n");
    }
    // 
    // AT_RESPONSE_OK;
    return HI_ERR_SUCCESS;
}

const at_cmd_func g_at_smart_home_platform_func_tbl[] = {
    {"+SMART_HOME_PLATFORM", 20, HI_NULL, HI_NULL, HI_NULL, (at_call_back_func)at_exe_smart_home_platform_cmd},
};

void smart_home_platform_entry(void)
{
    hi_at_register_cmd(g_at_smart_home_platform_func_tbl, sizeof(g_at_smart_home_platform_func_tbl) / sizeof(g_at_smart_home_platform_func_tbl[0]));
}
SYS_RUN(smart_home_platform_entry);
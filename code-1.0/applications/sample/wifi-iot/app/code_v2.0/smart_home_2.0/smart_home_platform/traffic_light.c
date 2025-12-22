
#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "oled_ssd1306.h"
#include "beeper_music.h"

void initLights(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);
}

WifiIotGpioValue parseChar(char c)
{
    if (c == '0')  //如果是字符'0'，返回低电平
    {
        return WIFI_IOT_GPIO_VALUE0;
    }
    else//其他返回高电平
    {
        return WIFI_IOT_GPIO_VALUE1;
    }
}

/**
 * cmd ：xxxxxx;//10000
 *        
 *      红：0表示灭，1表示亮   GPIO10
 *      绿：0表示灭，1表示亮    GPIO11
 *      黄：0表示灭，1表示亮    GPIO12
 * 
 *      beeper:0停止，1表示播放
 */
void  lights_controller(unsigned char *cmd)
{

    char line[32]={0};
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, parseChar(cmd[0])); //红   '1','0'--->0,1
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, parseChar(cmd[2])); //绿
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, parseChar(cmd[1]));//黄

    beep = cmd[3]-'0'; // char-‘0’,把字数转换为数字 ，‘0’--》0，‘1’--》1

    snprintf(line, sizeof(line), "red: %s",cmd[0]=='0'?"OFF":"ON " );
    OledShowString(0, 2, line, 1);

    snprintf(line, sizeof(line), "yellow: %s", cmd[2]=='0'?"OFF":"ON ");
    OledShowString(0, 3, line, 1);

    snprintf(line, sizeof(line), "green: %s", cmd[1]=='0'?"OFF":"ON ");
    OledShowString(0, 4, line, 1);
    snprintf(line, sizeof(line), "beep: %s", cmd[3]=='0'?"OFF":"ON ");
    OledShowString(0, 5, line, 1);
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "aht20.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"



static uint32_t AHT20_Read(uint8_t* buffer, uint32_t buffLen)
{
    WifiIotI2cData data = { 0 };
    data.receiveBuf = buffer;
    data.receiveLen = buffLen;
    uint32_t retval = I2cRead(AHT20_I2C_IDX, AHT20_READ_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) 
    {
        printf("I2cRead() failed, %0X!\n", retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_Write(uint8_t* buffer, uint32_t buffLen)
{
    WifiIotI2cData data = { 0 };
    data.sendBuf = buffer;
    data.sendLen = buffLen;
    uint32_t retval = I2cWrite(AHT20_I2C_IDX, AHT20_WRITE_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) 
    {
        printf("I2cWrite(%02X) failed, %0X!\n", buffer[0], retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

// 发送获取状态命令
static uint32_t AHT20_StatusCommand(void)
{
    uint8_t statusCmd[] = { AHT20_CMD_STATUS };
    return AHT20_Write(statusCmd, sizeof(statusCmd));
}

// 发送软复位命令
static uint32_t AHT20_ResetCommand(void)
{
    uint8_t resetCmd[] = {AHT20_CMD_RESET};
    return AHT20_Write(resetCmd, sizeof(resetCmd));
}

// 发送初始化校准命令
static uint32_t AHT20_CalibrateCommand(void)
{
    uint8_t clibrateCmd[] = {AHT20_CMD_CALIBRATION, AHT20_CMD_CALIBRATION_ARG0, AHT20_CMD_CALIBRATION_ARG1};
    return AHT20_Write(clibrateCmd, sizeof(clibrateCmd));
}

// 读取温湿度值之前， 首先要看状态字的校准使能位Bit[3]是否为 1(通过发送0x71可以获取一个字节的状态字)，
// 如果不为1，要发送0xBE命令(初始化)，此命令参数有两个字节， 第一个字节为0x08，第二个字节为0x00。
uint32_t AHT20_Calibrate(void)
{
    uint32_t retval = 0;
    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = { AHT20_CMD_STATUS };
    memset(&buffer, 0x0, sizeof(buffer));

    retval = AHT20_StatusCommand();
    if (retval != WIFI_IOT_SUCCESS) 
    {
        return retval;
    }

    retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) 
    {
        return retval;
    }

    if (AHT20_STATUS_BUSY(buffer[0]) || !AHT20_STATUS_CALI(buffer[0])) 
    {
        retval = AHT20_ResetCommand();
        if (retval != WIFI_IOT_SUCCESS) 
        {
            return retval;
        }
        usleep(AHT20_STARTUP_TIME);
        retval = AHT20_CalibrateCommand();
        usleep(AHT20_CALIBRATION_TIME);
        return retval;
    }

    return WIFI_IOT_SUCCESS;
}

// 发送 触发测量 命令，开始测量
uint32_t AHT20_StartMeasure(void)
{
    uint8_t triggerCmd[] = {AHT20_CMD_TRIGGER, AHT20_CMD_TRIGGER_ARG0, AHT20_CMD_TRIGGER_ARG1};
    return AHT20_Write(triggerCmd, sizeof(triggerCmd));
}

// 接收测量结果，拼接转换为标准值
uint32_t AHT20_GetMeasureResult(float* temp, float* humi)
{
    uint32_t retval = 0, i = 0;
    if (temp == NULL || humi == NULL) 
    {
        return WIFI_IOT_FAILURE;
    }

    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = { 0 };
    memset(&buffer, 0x0, sizeof(buffer));
    retval = AHT20_Read(buffer, sizeof(buffer));  // recv status command result
    if (retval != WIFI_IOT_SUCCESS) 
    {
        return retval;
    }

    for (i = 0; AHT20_STATUS_BUSY(buffer[0]) && i < AHT20_MAX_RETRY; i++) 
    {
        // printf("AHT20 device busy, retry %d/%d!\r\n", i, AHT20_MAX_RETRY);
        usleep(AHT20_MEASURE_TIME);
        retval = AHT20_Read(buffer, sizeof(buffer));  // recv status command result
        if (retval != WIFI_IOT_SUCCESS) 
        {
            return retval;
        }
    }
    if (i >= AHT20_MAX_RETRY) 
    {
        printf("AHT20 device always busy!\r\n");
        return WIFI_IOT_FAILURE;
    }

    uint32_t humiRaw = buffer[1];
    humiRaw = (humiRaw << 8) | buffer[2];
    humiRaw = (humiRaw << 4) | ((buffer[3] & 0xF0) >> 4);
    *humi = humiRaw / (float)AHT20_RESLUTION * 100;

    uint32_t tempRaw = buffer[3] & 0x0F;
    tempRaw = (tempRaw << 8) | buffer[4];
    tempRaw = (tempRaw << 8) | buffer[5];
    *temp = tempRaw / (float)AHT20_RESLUTION * 200 - 50;
    // printf("humi = %05X, %f, temp= %05X, %f\r\n", humiRaw, *humi, tempRaw, *temp);
    return WIFI_IOT_SUCCESS;
}

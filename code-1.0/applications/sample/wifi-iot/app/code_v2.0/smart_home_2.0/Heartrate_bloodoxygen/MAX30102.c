#include <stdio.h>
#include <unistd.h>
#include <securec.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "hos_types.h"
#include "MAX30102.h"
#include "blood.h"

uint16_t fifo_red;
uint16_t fifo_ir;
/*

*/
unsigned int IoTI2cWrite(WifiIotI2cIdx id, unsigned short deviceAddr, unsigned char *data, unsigned int dataLen)
{
	WifiIotI2cData i2cData;
	i2cData.receiveBuf = NULL;
	i2cData.receiveLen = 0;
	i2cData.sendBuf = data;
	i2cData.sendLen = dataLen;

	return I2cWrite(id, deviceAddr, &i2cData);
}

unsigned int IoTI2cRead(WifiIotI2cIdx id, unsigned short deviceAddr, unsigned char *data, unsigned int dataLen)
{
	WifiIotI2cData i2cData;
	i2cData.receiveBuf = data;
	i2cData.receiveLen = dataLen;
	i2cData.sendBuf = NULL;
	i2cData.sendLen = 0;

	return I2cRead(id, deviceAddr, &i2cData);
}

void MAX30102_GPIO(void)
{
	GpioInit();
	GpioSetDir(MAX30102_INT_GPIO, WIFI_IOT_GPIO_DIR_IN);
	IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
	IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

	I2cInit(MAX30102_I2C_IDX, MAX30102_I2C_BAUDRATE); // I2C初始化
}

static unsigned int maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *data, uint16_t len)
{
	unsigned int ret = 0;
	ret += IoTI2cWrite(MAX30102_I2C_IDX, (MAX30102_ADDR << 1) | 0, &uch_addr, 1);
	ret += IoTI2cRead(MAX30102_I2C_IDX, (MAX30102_ADDR << 1) | 1, data, len);
	if (ret)
	{
		printf("max30102 read falled.\n");
	}
	return ret;
}

static unsigned int maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
	uint8_t buffer[2] = {uch_addr, uch_data};
	if (IoTI2cWrite(MAX30102_I2C_IDX, (MAX30102_ADDR << 1) | 0, buffer, ARRAY_SIZE(buffer)))
	{
		printf("max30102 write falled.\n");
	}
	else
	{
		printf("max30102 write OK.\n");
	}
	return 0;
}

uint8_t Max30102_reset(void)
{
	if (maxim_max30102_write_reg(REG_MODE_CONFIG, 0x40))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void MAX30102_Config(void)
{
	maxim_max30102_write_reg(REG_INTR_ENABLE_1, 0xc0); // INTR setting
	maxim_max30102_write_reg(REG_INTR_ENABLE_2, 0x00); //
	maxim_max30102_write_reg(REG_FIFO_WR_PTR, 0x00);   // FIFO_WR_PTR[4:0]
	maxim_max30102_write_reg(REG_OVF_COUNTER, 0x00);   // OVF_COUNTER[4:0]
	maxim_max30102_write_reg(REG_FIFO_RD_PTR, 0x00);   // FIFO_RD_PTR[4:0]

	maxim_max30102_write_reg(REG_FIFO_CONFIG, 0x0f); // sample avg = 1, fifo rollover=false, fifo almost full = 17
	maxim_max30102_write_reg(REG_MODE_CONFIG, 0x03); // 0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	maxim_max30102_write_reg(REG_SPO2_CONFIG, 0x27); // SPO2_ADC range = 4096nA, SPO2 sample rate (50 Hz), LED pulseWidth (400uS)
	maxim_max30102_write_reg(REG_LED1_PA, 0x32);	 // Choose value for ~ 10mA for LED1
	maxim_max30102_write_reg(REG_LED2_PA, 0x32);	 // Choose value for ~ 10mA for LED2
	maxim_max30102_write_reg(REG_PILOT_PA, 0x7f);	 // Choose value for ~ 25mA for Pilot LED
}

void max30102_read_fifo(void)
{
	uint16_t un_temp;
	fifo_red = 0;
	fifo_ir = 0;
	uint8_t ach_i2c_data[6];
	uint8_t uch_temp;
	unsigned int ret = 0;

	ret += maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp, sizeof(uch_temp));
	ret += maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp, sizeof(uch_temp));
	ret += maxim_max30102_read_reg(REG_FIFO_DATA, ach_i2c_data, 6);
	if (ret)
	{
		printf("max30102 read fifo failed.\n");
	}

	un_temp = ach_i2c_data[0];
	un_temp <<= 14;
	fifo_red += un_temp;
	un_temp = ach_i2c_data[1];
	un_temp <<= 6;
	fifo_red += un_temp;
	un_temp = ach_i2c_data[2];
	un_temp >>= 2;
	fifo_red += un_temp;

	un_temp = ach_i2c_data[3];
	un_temp <<= 14;
	fifo_ir += un_temp;
	un_temp = ach_i2c_data[4];
	un_temp <<= 6;
	fifo_ir += un_temp;
	un_temp = ach_i2c_data[5];
	un_temp >>= 2;
	fifo_ir += un_temp;

	if (fifo_ir <= 10000)
	{
		fifo_ir = 0;
	}
	if (fifo_red <= 10000)
	{
		fifo_red = 0;
	}
}

void MAX30102Task(void *arg)
{
	(void)arg;
	uint8_t id;
	int i = 0;
	WifiIotGpioValue value;

	MAX30102_GPIO();
	maxim_max30102_read_reg(REG_PART_ID, &id, 1);
	printf("\r\n id = %d \r\n", id);

	Max30102_reset();
	MAX30102_Config();
	for (i = 0; i < 128; i++)
	{
		do
		{
			GpioGetInputVal(MAX30102_INT_GPIO, &value);
		} while (value == WIFI_IOT_GPIO_VALUE1);

		// 读取FIFO
		max30102_read_fifo();
	}
	while (1)
	{
		blood_Loop();
		usleep(100);
	}
}

void MAX30102Demo(void)
{
	osThreadAttr_t attr;

	attr.name = "MAX30102Task";
	attr.attr_bits = 0U;
	attr.cb_mem = NULL;
	attr.cb_size = 0U;
	attr.stack_mem = NULL;
	attr.stack_size = MAX30102_TASK_STACK_SIZE;
	attr.priority = MAX30102_TASK_PRIO;

	if (osThreadNew((osThreadFunc_t)MAX30102Task, NULL, &attr) == NULL)
	{
		printf("\r\n[MAX30102Demo] Falied to create MAX30102Task!\n");
	}
}

APP_FEATURE_INIT(MAX30102Demo);
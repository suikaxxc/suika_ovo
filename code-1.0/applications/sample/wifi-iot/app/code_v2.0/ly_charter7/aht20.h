

#ifndef AHT20_H
#define AHT20_H

#include <stdint.h>

#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

#define AHT20_STARTUP_TIME     20*1000 // 上电启动时间
#define AHT20_CALIBRATION_TIME 40*1000 // 初始化（校准）时间
#define AHT20_MEASURE_TIME     75*1000 // 测量时间

#define AHT20_DEVICE_ADDR   0x38        //设备地址
#define AHT20_READ_ADDR     ((0x38<<1)|0x1)
#define AHT20_WRITE_ADDR    ((0x38<<1)|0x0)

#define AHT20_CMD_CALIBRATION       0xBE // 初始化（校准）命令
#define AHT20_CMD_CALIBRATION_ARG0  0x08
#define AHT20_CMD_CALIBRATION_ARG1  0x00

/**
 * 传感器在采集时需要时间,主机发出测量指令（0xAC）后，延时75毫秒以上再读取转换后的数据并判断返回的状态位是否正常。
 * 若状态比特位[Bit7]为0代表数据可正常读取，为1时传感器为忙状态，主机需要等待数据处理完成。
 **/
#define AHT20_CMD_TRIGGER       0xAC // 触发测量命令
#define AHT20_CMD_TRIGGER_ARG0  0x33
#define AHT20_CMD_TRIGGER_ARG1  0x00

// 用于在无需关闭和再次打开电源的情况下，重新启动传感器系统，软复位所需时间不超过20 毫秒
#define AHT20_CMD_RESET      0xBA // 软复位命令

#define AHT20_CMD_STATUS     0x71 // 获取状态命令

/**
 * STATUS 命令回复：
 * 1. 初始化后触发测量之前，STATUS 只回复 1B 状态值；
 * 2. 触发测量之后，STATUS 回复6B： 1B 状态值 + 2B 湿度 + 4b湿度 + 4b温度 + 2B 温度
 *      RH = Srh / 2^20 * 100%
 *      T  = St  / 2^20 * 200 - 50
 **/
#define AHT20_STATUS_BUSY_SHIFT 7       // bit[7] Busy indication
#define AHT20_STATUS_BUSY_MASK  (0x1<<AHT20_STATUS_BUSY_SHIFT)
#define AHT20_STATUS_BUSY(status) ((status & AHT20_STATUS_BUSY_MASK) >> AHT20_STATUS_BUSY_SHIFT)

#define AHT20_STATUS_MODE_SHIFT 5       // bit[6:5] Mode Status
#define AHT20_STATUS_MODE_MASK  (0x3<<AHT20_STATUS_MODE_SHIFT)
#define AHT20_STATUS_MODE(status) ((status & AHT20_STATUS_MODE_MASK) >> AHT20_STATUS_MODE_SHIFT)

                                        // bit[4] Reserved
#define AHT20_STATUS_CALI_SHIFT 3       // bit[3] CAL Enable
#define AHT20_STATUS_CALI_MASK  (0x1<<AHT20_STATUS_CALI_SHIFT)
#define AHT20_STATUS_CALI(status) ((status & AHT20_STATUS_CALI_MASK) >> AHT20_STATUS_CALI_SHIFT)
                                        // bit[2:0] Reserved

#define AHT20_STATUS_RESPONSE_MAX 6

#define AHT20_RESLUTION            (1<<20)  // 2^20

#define AHT20_MAX_RETRY 10

// 发送初始化校准命令
uint32_t AHT20_Calibrate(void);
// 发送 触发测量 命令，开始测量
uint32_t AHT20_StartMeasure(void);
// 接收测量结果，拼接转换为标准值
uint32_t AHT20_GetMeasureResult(float* temp, float* humi);

#endif  // AHT20_H
# 智能水生植物养殖缸系统 - 硬件端

## 项目概述

本项目是基于Hi3861的智能水生植物养殖缸系统的硬件控制端代码。该系统通过多种传感器实时监测水位、水温、水质和光照，并自动控制水泵、加热器、风扇和LED补光灯，实现智能化的水生植物养护。

## 功能特性

### 1. 智能自动补水功能
- 使用YW001水位传感器监测水位
- 水位低于下限时自动启动补水泵
- 水位达到上限后自动停止
- 杜绝干烧风险

### 2. 智能水体温控功能
- 使用DS18B20测量水温
- 水温过低时启动加热片加热
- 水温过高时启动PWM风扇散热
- 将水温维持在最适区间

### 3. 智能LED补光功能
- 使用光敏电阻(LDR)检测环境光
- 光照不足时自动开启LED补光
- 根据植物需求设置光照阈值

### 4. 报警功能
- 使用蜂鸣器发出警报
- 水位极低时报警(防止干烧)
- 温度异常时报警
- 水质TDS过高时报警

### 5. Wi-Fi联网
- Hi3861自动连接路由器
- 获取DHCP分配的IP地址

### 6. 手机远程控制
- 通过MQTT与HarmonyOS手机App通信
- 支持远程手动控制各设备
- 支持自动/手动模式切换

### 7. AI智能养护策略
- 支持8种水生植物预设参数
- 用户可通过App选择植物品种
- 系统自动匹配最优生长参数

## 硬件连接

### 传感器

| 传感器 | 引脚 | 功能说明 |
|--------|------|----------|
| YW001水位传感器 | GPIO07/ADC3 | 水位检测 |
| DS18B20温度传感器 | GPIO08 | 水温检测(1-Wire)，注意：不能使用GPIO02，会与UART0冲突 |
| TDS水质传感器 | GPIO11/ADC5 | 水质TDS检测 |
| 浊度传感器 | GPIO01/ADC1 | 水体浊度检测(NTU) |
| 光敏电阻(LDR) | GPIO12/ADC0 | 环境光检测(返回lux单位) |

### 执行器

| 执行器 | 引脚 | 功能说明 |
|--------|------|----------|
| 抽水泵继电器(高电平触发) | GPIO00 | 抽水泵开关控制 |
| 补水泵继电器(高电平触发) | GPIO05 | 补水泵开关控制（GPIO05从按键功能改为继电器控制） |
| 加热片 | GPIO10 | 加热控制 |
| PWM风扇 | GPIO04/PWM1 | 散热控制(PWM调速，支持0-100%速度控制) |
| LED补光灯 | GPIO03 | 补光控制 |
| 蜂鸣器 | GPIO09/PWM0 | 报警输出 |

### 显示与输入

| 设备 | 引脚 | 功能说明 |
|------|------|----------|
| OLED显示屏(I2C) | GPIO13(SDA), GPIO14(SCL) | 本地数据显示 |

**注意**: GPIO05原用于按键S1/S2功能，已改为补水泵继电器控制引脚。按键功能已移除，OLED显示页面改为自动循环切换。

### GPIO引脚使用总览

| GPIO | 功能 | 备注 |
|------|------|------|
| GPIO00 | 抽水泵继电器 | 高电平触发 |
| GPIO01 | 浊度传感器 | ADC1输入 |
| GPIO02 | UART0_TX | 系统调试串口，不可使用 |
| GPIO03 | LED补光灯 | 数字输出控制 |
| GPIO04 | PWM风扇 | PWM1调速输出(0-100%) |
| GPIO05 | 补水泵继电器 | 高电平触发(原按键功能已移除) |
| GPIO06 | 预留 | 未使用 |
| GPIO07 | 水位传感器 | ADC3输入 |
| GPIO08 | DS18B20温度传感器 | 1-Wire协议 |
| GPIO09 | 蜂鸣器 | PWM0音频输出 |
| GPIO10 | 加热片 | 数字输出控制 |
| GPIO11 | TDS水质传感器 | ADC5输入 |
| GPIO12 | 光敏电阻(LDR) | ADC0输入 |
| GPIO13 | OLED I2C SDA | I2C0数据线 |
| GPIO14 | OLED I2C SCL | I2C0时钟线 |

## 软件架构

```
suika_demo/
├── main.c              # 主入口，任务调度
├── i2c_common.c/h      # I2C通用初始化
├── water_level.c/h     # 水位传感器模块
├── ds18b20.c/h         # 温度传感器模块
├── tds_sensor.c/h      # TDS水质传感器模块
├── turbidity_sensor.c/h # 浊度传感器模块
├── light_sensor.c/h    # 光照传感器模块
├── pump_control.c/h    # 水泵控制模块
├── temp_control.c/h    # 温度控制模块(加热/散热)
├── led_control.c/h     # LED控制模块
├── alarm.c/h           # 报警控制模块
├── tank_control.c/h    # 核心控制逻辑
├── oled_display.c/h    # OLED显示模块
├── oled_ssd1306.c/h    # OLED驱动
├── oled_fonts.h        # OLED字体
├── wifi_connect.c/h    # WiFi连接模块
├── mqtt_client.c/h     # MQTT通信模块
└── BUILD.gn            # 构建配置
```

## MQTT通信协议

### 发布主题: `tank/data`
设备定期发布传感器数据，JSON格式：
```json
{
  "waterLevel": 65,
  "waterTemp": 24.5,
  "lightIntensity": 45,
  "tdsValue": 280,
  "turbidityValue": 120,
  "pumpStatus": 0,
  "waterPumpStatus": 0,
  "heaterStatus": 0,
  "fanSpeed": 0,
  "ledStatus": 1,
  "alarmStatus": 0,
  "alarmMessage": ""
}
```

### 订阅主题: `tank/control`
接收来自App的控制命令，JSON格式：
```json
{"type": "led", "value": 1, "timestamp": 1234567890}
{"type": "pump", "value": 1, "timestamp": 1234567890}
{"type": "waterPump", "value": 1, "timestamp": 1234567890}
{"type": "heater", "value": 1, "timestamp": 1234567890}
{"type": "fan", "value": 50, "timestamp": 1234567890}
{"type": "mode", "value": 0, "timestamp": 1234567890}
{"type": "plant", "value": 0, "timestamp": 1234567890}
{"type": "settings", "waterTempMin": 20, "waterTempMax": 28, ...}
```

## 预设植物参数

| 植物 | 水温范围(°C) | 水位范围(%) | 光照阈值(%) | 补光时长(h) |
|------|--------------|-------------|-------------|-------------|
| 绿萝 | 18-28 | 20-60 | 30 | 8 |
| 睡莲 | 22-30 | 50-90 | 80 | 10 |
| 铜钱草 | 16-26 | 30-70 | 40 | 6 |
| 水葫芦 | 20-32 | 40-80 | 60 | 8 |
| 富贵竹 | 18-25 | 25-50 | 20 | 6 |
| 碗莲 | 20-28 | 45-85 | 70 | 10 |
| 水仙 | 10-18 | 20-40 | 50 | 8 |
| 风信子 | 12-20 | 15-35 | 40 | 6 |

## 配置说明

### WiFi配置
在 `wifi_connect.c` 中修改：
```c
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
```

### MQTT配置
在 `mqtt_client.c` 中修改：
```c
#define MQTT_HOST "192.168.x.x"  // MQTT Broker地址
#define MQTT_PORT 1883
```

## 编译说明

1. 确保已正确配置OpenHarmony/LiteOS编译环境
2. 在 `applications/sample/wifi-iot/app/BUILD.gn` 中添加本模块
3. 执行编译命令: `python build.py wifiiot`

## 与HarmonyOS App联动

本硬件代码与同仓库的 `suika-demo001-copilot-update-code-for-harmonyos` 目录下的HarmonyOS App配合使用。App提供：
- 实时传感器数据显示
- 设备手动控制
- 植物品种选择
- AI养护建议

## 注意事项

1. DS18B20使用单总线协议，需要4.7kΩ上拉电阻
2. 继电器模块建议使用独立电源，并与Hi3861共地
3. 蜂鸣器使用PWM驱动，共振频率约2700Hz
4. OLED与温湿度传感器共用I2C0，使用互斥锁保护

## 版本信息

- 版本: 1.0
- 平台: Hi3861 + HarmonyOS/LiteOS
- 作者: suikaxxc

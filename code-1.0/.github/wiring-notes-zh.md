# WiFi-IoT 传感器与执行器接线说明（ly_charter7）

> 面向 Hi3861/HarmonyOS WiFi IoT 开发板（以 Pegasus 套件为例）。若您的板卡丝印或针脚定义不同，请以实际丝印为准，并在代码中调整 GPIO/ADC 定义。

## 本示例用到的引脚
- I2C0：SDA=GPIO13，SCL=GPIO14（驱动 0.96" OLED）。
- 按键（页面切换）：GPIO5，内部上拉，按下接地。
- LDR 光敏传感器（ADC0）：WIFI_IOT_ADC_CHANNEL_0，对应板上 ADC0 引脚（请查丝印，多数板子标为 "ADC0" / "IO_*/ADC"）。
- LED（受 LDR 控制）：GPIO9，推挽输出，高电平点亮（蜂鸣器若启用会占用 PWM0/GPIO9，请勿同时使用）。
- 蜂鸣器：PWM0/ GPIO9（已有示例）。
- MQ2 可燃气体传感器：ADC5（已有示例）。
- 液位传感器 YW01：请按原示例接线。

## 具体接线步骤
1) **OLED (I2C)**
   - OLED SDA → GPIO13
   - OLED SCL → GPIO14
   - VCC → 3.3V, GND → GND

2) **按键（OLED 页面切换）**
   - 一端接 GPIO5
   - 另一端接 GND
   - 代码已开启内部上拉，无需外接上拉；若需防抖，可在硬件侧串 0.1?F 电容并联到 GND。

3) **LDR 光敏传感器（分压接入 ADC0）**
   - 采用 LDR 与 10kΩ 电阻分压：
     - 3.3V → LDR → 分压节点 → 10kΩ → GND
     - 分压节点 → ADC0 引脚（WIFI_IOT_ADC_CHANNEL_0 对应的 ADC0/丝印管脚）
   - 若光线变暗，ADC 电压上升/下降取决于接法；本代码按百分比阈值 30% 判断（`Get_LightPercent()`）。如方向相反可交换 LDR 与电阻位置。

4) **LED（受 LDR 阈值控制）**
   - GPIO9 → LED → 220Ω 电阻 → GND（或 GPIO9 → 220Ω → LED → GND），高电平点亮。
   - 阈值逻辑：光照百分比 < 30% 点亮，否则熄灭。

5) **蜂鸣器（原示例）**
   - PWM0/ GPIO9 → 有源蜂鸣器正极，蜂鸣器负极 → GND。

6) **MQ2（原示例）**
   - MQ2 模块的 AO → ADC5 引脚（WIFI_IOT_ADC_CHANNEL_5 对应丝印）。
   - VCC→3.3V，GND→GND。

## 代码中的关键定义（如需改针脚）
- LED：`ldr_led_control.c` 中 `LDR_LED_GPIO`/`LDR_LED_IO_NAME`（默认 GPIO10）。
- 按键：`oled_demo.c` 中 `OLED_BTN_GPIO`/`OLED_BTN_IO_NAME`（默认 GPIO11）。
- LDR 通道：`ldr_demo.c` / `LDR_TASK` 使用 `WIFI_IOT_ADC_CHANNEL_0`。
- OLED：`oled_ssd1306.c` I2C0 → GPIO13/14。

## 调试提示
- 若 OLED 无显示，先确认 I2C0 线序与 3.3V 供电正确。
- 若按键无效，检查是否与 GND 正确短接、或尝试外接 10kΩ 上拉至 3.3V。
- 若 LED 反相，可在代码中反转 `turnOn` 判断或调换 LED/电阻顺序。
- 若光敏百分比波动大，可在 `LdrLed_Task` 中增加平均滤波或延长采样周期。

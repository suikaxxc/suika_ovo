import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import { ControlModeArray } from "../common/constants/Constant";
import { promptAction } from '@kit.ArkUI';

// 默认风扇速度（百分比）
const DEFAULT_FAN_SPEED: number = 50;

@Component
export struct ControlComponent {
  @Link
  item: AquaticTankViewModel;

  // 回调函数不能使用@Prop装饰器，直接作为普通属性传递
  onControlChange: (controlType: string, value: number) => void = () => {};

  // 用于防止Toggle onChange被MQTT同步触发时发送重复命令
  // 记录上次已知的模式值（从item.controlMode或上次发送的值）
  private lastKnownControlMode: number = -1;

  build() {
    Scroll() {
      Column() {
        // 标题和模式切换
        Row() {
          Text("设备控制")
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
          Blank()
          Row() {
            Text(ControlModeArray[this.item.controlMode])
              .fontSize(14)
              .fontColor(this.item.controlMode === 0 ? '#42A5F5' : '#1976D2')
            Toggle({ type: ToggleType.Switch, isOn: this.item.controlMode === 1 })
              .width(50)
              .height(26)
              .selectedColor('#42A5F5')
              .switchPointColor(Color.White)
              .margin({ left: 8 })
              .onChange((isOn: boolean) => {
                const newMode = isOn ? 1 : 0;
                const currentItemMode = this.item.controlMode;
                
                // 只有当用户真正切换了模式时才发送命令
                // 检查：新模式与当前item.controlMode不同（说明是用户操作）
                // 而不是MQTT同步后再次触发onChange（此时item.controlMode已经等于newMode）
                if (newMode !== currentItemMode) {
                  // 用户正在切换模式
                  this.item.controlMode = newMode;
                  this.lastKnownControlMode = newMode;
                  this.onControlChange('mode', newMode);
                  promptAction.showToast({
                    message: `已切换为${ControlModeArray[newMode]}`,
                    duration: 1500
                  });
                  console.info(`[ControlComponent] 模式切换命令已发送: ${newMode === 1 ? '手动' : '自动'}`);
                } else {
                  // onChange被MQTT同步触发，item.controlMode已经是newMode了
                  // 只更新lastKnownControlMode，不发送命令
                  this.lastKnownControlMode = newMode;
                  console.info(`[ControlComponent] 忽略同步触发的onChange: mode=${newMode}`);
                }
              })
          }
        }
        .width('100%')
        .margin({ bottom: 16 })

        // 控制按钮区域
        Column({ space: 12 }) {
        // LED灯控制
        Row() {
          Row() {
            Image($r('app.media.led'))
              .width(28)
              .height(28)
            Column() {
              Text("LED补光灯")
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .fontColor('#333333')
              Text(this.item.ledStatus === 1 ? "补光中" : "已关闭")
                .fontSize(12)
                .fontColor(this.item.ledStatus === 1 ? '#FFC107' : '#999999')
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 12 })
          }

          Blank()

          Button(this.item.ledStatus === 1 ? "关闭" : "开启")
            .fontSize(14)
            .fontColor(this.item.ledStatus === 1 ? '#333333' : Color.White)
            .backgroundColor(this.item.ledStatus === 1 ? '#757575' : '#FFC107')
            .height(32)
            .width(70)
            .borderRadius(16)
            .enabled(this.item.controlMode === 1)
            .opacity(this.item.controlMode === 1 ? 1 : 0.5)
            .onClick(() => {
              this.toggleDevice('led');
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#FFF8E1')
        .borderRadius(10)

        // 补水控制
        Row() {
          Row() {
            Image($r('app.media.pump'))
              .width(28)
              .height(28)
            Column() {
              Text("补水泵")
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .fontColor('#333333')
              Text(this.item.pumpStatus === 1 ? "运行中" : "已停止")
                .fontSize(12)
                .fontColor(this.item.pumpStatus === 1 ? '#2196F3' : '#999999')
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 12 })
          }

          Blank()

          Button(this.item.pumpStatus === 1 ? "停止" : "启动")
            .fontSize(14)
            .fontColor(Color.White)
            .backgroundColor(this.item.pumpStatus === 1 ? '#FF5722' : '#2196F3')
            .height(32)
            .width(70)
            .borderRadius(16)
            .enabled(this.item.controlMode === 1)
            .opacity(this.item.controlMode === 1 ? 1 : 0.5)
            .onClick(() => {
              this.toggleDevice('pump');
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#E3F2FD')
        .borderRadius(10)

        // 抽水泵控制
        Row() {
          Row() {
            Image($r('app.media.pump'))
              .width(28)
              .height(28)
            Column() {
              Text("抽水泵")
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .fontColor('#333333')
              Text(this.item.waterPumpStatus === 1 ? "运行中" : "已停止")
                .fontSize(12)
                .fontColor(this.item.waterPumpStatus === 1 ? '#9C27B0' : '#999999')
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 12 })
          }

          Blank()

          Button(this.item.waterPumpStatus === 1 ? "停止" : "启动")
            .fontSize(14)
            .fontColor(Color.White)
            .backgroundColor(this.item.waterPumpStatus === 1 ? '#FF5722' : '#9C27B0')
            .height(32)
            .width(70)
            .borderRadius(16)
            .enabled(this.item.controlMode === 1)
            .opacity(this.item.controlMode === 1 ? 1 : 0.5)
            .onClick(() => {
              this.toggleDevice('waterPump');
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#F3E5F5')
        .borderRadius(10)

        // 加热器控制
        Row() {
          Row() {
            Image($r('app.media.heater'))
              .width(28)
              .height(28)
            Column() {
              Text("加热器")
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .fontColor('#333333')
              Text(this.item.heaterStatus === 1 ? "加热中" : "已关闭")
                .fontSize(12)
                .fontColor(this.item.heaterStatus === 1 ? '#FF5722' : '#999999')
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 12 })
          }

          Blank()

          Button(this.item.heaterStatus === 1 ? "关闭" : "开启")
            .fontSize(14)
            .fontColor(Color.White)
            .backgroundColor(this.item.heaterStatus === 1 ? '#757575' : '#FF5722')
            .height(32)
            .width(70)
            .borderRadius(16)
            .enabled(this.item.controlMode === 1)
            .opacity(this.item.controlMode === 1 ? 1 : 0.5)
            .onClick(() => {
              this.toggleDevice('heater');
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#FBE9E7')
        .borderRadius(10)

        // 散热风扇控制（PWM调速）
        Column() {
          Row() {
            Row() {
              Image($r('app.media.fan'))
                .width(28)
                .height(28)
              Column() {
                Text("散热风扇")
                  .fontSize(14)
                  .fontWeight(FontWeight.Medium)
                  .fontColor('#333333')
                Text(this.item.fanSpeed > 0 ? `${this.item.fanSpeed}%` : "已关闭")
                  .fontSize(12)
                  .fontColor(this.item.fanSpeed > 0 ? '#00BCD4' : '#999999')
              }
              .alignItems(HorizontalAlign.Start)
              .margin({ left: 12 })
            }

            Blank()

            Button(this.item.fanSpeed > 0 ? "关闭" : "开启")
              .fontSize(14)
              .fontColor(Color.White)
              .backgroundColor(this.item.fanSpeed > 0 ? '#757575' : '#00BCD4')
              .height(32)
              .width(70)
              .borderRadius(16)
              .enabled(this.item.controlMode === 1)
              .opacity(this.item.controlMode === 1 ? 1 : 0.5)
              .onClick(() => {
                if (this.item.fanSpeed > 0) {
                  this.item.fanSpeed = 0;
                } else {
                  this.item.fanSpeed = DEFAULT_FAN_SPEED;
                }
                this.onControlChange('fan', this.item.fanSpeed);
              })
          }
          .width('100%')

          // 风扇速度滑块控制
          Row() {
            Text("0%")
              .fontSize(10)
              .fontColor('#999999')
            Slider({
              value: this.item.fanSpeed,
              min: 0,
              max: 100,
              step: 5,
              style: SliderStyle.OutSet
            })
              .layoutWeight(1)
              .blockColor('#00BCD4')
              .trackColor('#E0E0E0')
              .selectedColor('#00BCD4')
              .showTips(true)
              .enabled(this.item.controlMode === 1)
              .opacity(this.item.controlMode === 1 ? 1 : 0.5)
              .onChange((value: number, mode: SliderChangeMode) => {
                this.item.fanSpeed = Math.round(value);
                if (mode === SliderChangeMode.End) {
                  this.onControlChange('fan', this.item.fanSpeed);
                }
              })
            Text("100%")
              .fontSize(10)
              .fontColor('#999999')
          }
          .width('100%')
          .margin({ top: 8 })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#E0F7FA')
        .borderRadius(10)

        // OLED翻页按钮
        Row() {
          Row() {
            Image($r('app.media.settings'))
              .width(28)
              .height(28)
            Column() {
              Text("OLED显示屏")
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .fontColor('#333333')
              Text("点击切换显示页面")
                .fontSize(12)
                .fontColor('#999999')
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 12 })
          }

          Blank()

          Button("翻页")
            .fontSize(14)
            .fontColor(Color.White)
            .backgroundColor('#607D8B')
            .height(32)
            .width(70)
            .borderRadius(16)
            .onClick(() => {
              this.onControlChange('oledPage', 1);
              promptAction.showToast({
                message: '已发送OLED翻页指令',
                duration: 1000
              });
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#ECEFF1')
        .borderRadius(10)
      }

      // 手动模式提示
      if (this.item.controlMode === 0) {
        Text("当前为自动模式，设备由系统自动控制")
          .fontSize(12)
          .fontColor('#999999')
          .margin({ top: 12 })
      }
    }
    .padding(14)
    .width('94%')
    .borderRadius(14)
    .backgroundColor(Color.White)
    .margin({ left: '3%', right: '3%' })
    .shadow({ radius: 8, color: '#1A000000', offsetX: 0, offsetY: 2 })
    }
    .scrollBar(BarState.Auto)
    .scrollable(ScrollDirection.Vertical)
    .width('100%')
    .height('100%')
  }

  toggleDevice(deviceType: string) {
    switch (deviceType) {
      case 'pump':
        this.item.pumpStatus = this.item.pumpStatus === 1 ? 0 : 1;
        this.onControlChange('pump', this.item.pumpStatus);
        break;
      case 'waterPump':
        this.item.waterPumpStatus = this.item.waterPumpStatus === 1 ? 0 : 1;
        this.onControlChange('waterPump', this.item.waterPumpStatus);
        break;
      case 'heater':
        this.item.heaterStatus = this.item.heaterStatus === 1 ? 0 : 1;
        this.onControlChange('heater', this.item.heaterStatus);
        break;
      case 'led':
        this.item.ledStatus = this.item.ledStatus === 1 ? 0 : 1;
        this.onControlChange('led', this.item.ledStatus);
        break;
    }
  }
}

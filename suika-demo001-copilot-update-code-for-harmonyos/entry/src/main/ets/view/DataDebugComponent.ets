import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import { promptAction } from '@kit.ArkUI';

@Component
export struct DataDebugComponent {
  @Link
  item: AquaticTankViewModel;

  @Prop
  manualDataMode: boolean = false;

  onModeChange: (enabled: boolean) => void = () => {};
  onValueApply: (tds: number, turbidity: number) => void = () => {};

  @State editTdsValue: number = 280;
  @State editTurbidity: number = 120;

  aboutToAppear(): void {
    this.editTdsValue = this.item.tdsValue;
    this.editTurbidity = this.item.turbidity;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Row() {
          Text("数据调试")
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
          Blank()
          Row() {
            Text(this.manualDataMode ? "手动数据模式" : "真实传感器模式")
              .fontSize(12)
              .fontColor(this.manualDataMode ? '#FF5722' : '#4CAF50')
            Toggle({ type: ToggleType.Switch, isOn: this.manualDataMode })
              .width(50)
              .height(26)
              .selectedColor('#FF5722')
              .switchPointColor(Color.White)
              .margin({ left: 8 })
              .onChange((isOn: boolean) => {
                this.onModeChange(isOn);
                promptAction.showToast({
                  message: isOn ? '已开启手动数据模式' : '已切回真实传感器模式',
                  duration: 1200
                });
              })
          }
        }
        .width('100%')

        Text("开启后，硬件将使用你设置的 ppm/NTU 值，并参与告警判断。")
          .fontSize(11)
          .fontColor('#999999')
          .width('100%')

        Column({ space: 8 }) {
          Text(`TDS (ppm): ${this.editTdsValue}`)
            .fontSize(14)
            .fontColor('#333333')
            .width('100%')
          Slider({
            value: this.editTdsValue,
            min: 0,
            max: 1000,
            step: 1,
            style: SliderStyle.OutSet
          })
            .blockColor('#42A5F5')
            .trackColor('#E0E0E0')
            .selectedColor('#42A5F5')
            .showTips(true)
            .onChange((value: number) => {
              this.editTdsValue = Math.round(value);
            })
            .width('100%')
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#E3F2FD')
        .borderRadius(10)

        Column({ space: 8 }) {
          Text(`浊度 (NTU): ${this.editTurbidity}`)
            .fontSize(14)
            .fontColor('#333333')
            .width('100%')
          Slider({
            value: this.editTurbidity,
            min: 0,
            max: 1000,
            step: 1,
            style: SliderStyle.OutSet
          })
            .blockColor('#FF9800')
            .trackColor('#E0E0E0')
            .selectedColor('#FF9800')
            .showTips(true)
            .onChange((value: number) => {
              this.editTurbidity = Math.round(value);
            })
            .width('100%')
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#FFF3E0')
        .borderRadius(10)

        Button("应用到硬件")
          .fontSize(14)
          .fontColor(Color.White)
          .backgroundColor(this.manualDataMode ? '#FF5722' : '#BDBDBD')
          .height(36)
          .width('100%')
          .borderRadius(18)
          .onClick(() => {
            if (!this.manualDataMode) {
              promptAction.showToast({
                message: '请先开启手动数据模式',
                duration: 1200
              });
              return;
            }
            this.onValueApply(this.editTdsValue, this.editTurbidity);
            promptAction.showToast({
              message: '已下发 ppm/NTU 手动值',
              duration: 1200
            });
          })
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
}

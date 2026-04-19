import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import { promptAction } from '@kit.ArkUI';

@Component
export struct DataDebugComponent {
  @Link
  item: AquaticTankViewModel;

  @Link
  manualDataMode: boolean;

  onScrollOffsetChange: (offsetY: number) => void = () => {};

  build() {
    Scroll() {
      Column({ space: 12 }) {
        Text("数据调试")
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor('#333333')
          .width('100%')

        Row() {
          Text("手动演示模式")
            .fontSize(14)
            .fontColor('#333333')
          Blank()
          Toggle({ type: ToggleType.Switch, isOn: this.manualDataMode })
            .selectedColor('#42A5F5')
            .switchPointColor(Color.White)
            .onChange((isOn: boolean) => {
              this.manualDataMode = isOn;
              promptAction.showToast({
                message: isOn ? '已开启手动演示模式' : '已关闭手动演示模式',
                duration: 1200
              });
            })
        }
        .padding(12)
        .backgroundColor('#F5F8FF')
        .borderRadius(10)

        this.SensorSliderCard('水位 (mm)', this.item.waterLevel, 0, 90, (v: number) => this.item.waterLevel = v)
        this.SensorSliderCard('水温 (°C)', this.item.waterTemp, 0, 50, (v: number) => this.item.waterTemp = v)
        this.SensorSliderCard('水质 (ppm)', this.item.tdsValue, 0, 2000, (v: number) => this.item.tdsValue = v)
        this.SensorSliderCard('光照 (lux)', this.item.lightIntensity, 0, 5000, (v: number) => this.item.lightIntensity = v)
        this.SensorSliderCard('浊度 (NTU)', this.item.turbidityValue, 0, 1000, (v: number) => this.item.turbidityValue = v)

        Button("恢复默认演示值")
          .width('100%')
          .height(40)
          .borderRadius(20)
          .fontSize(14)
          .fontColor(Color.White)
          .backgroundColor('#42A5F5')
          .onClick(() => {
            this.item.waterLevel = 50;
            this.item.waterTemp = 24;
            this.item.tdsValue = 280;
            this.item.lightIntensity = 450;
            this.item.turbidityValue = 120;
            promptAction.showToast({
              message: '已恢复默认演示数据',
              duration: 1200
            });
          })

        Text("说明：开启手动演示模式后，可直接修改页面显示数据，便于论文演示。")
          .fontSize(12)
          .fontColor('#999999')
          .width('100%')
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
    .onDidScroll((xOffset: number, yOffset: number) => {
      void xOffset;
      this.onScrollOffsetChange(yOffset);
    })
    .width('100%')
    .height('100%')
  }

  @Builder
  SensorSliderCard(title: string, value: number, min: number, max: number, onChangeValue: (v: number) => void) {
    Column() {
      Row() {
        Text(title)
          .fontSize(13)
          .fontColor('#333333')
        Blank()
        Text(`${Math.round(value)}`)
          .fontSize(13)
          .fontWeight(FontWeight.Medium)
          .fontColor('#42A5F5')
      }
      .width('100%')

      Slider({
        value: value,
        min: min,
        max: max,
        step: 1,
        style: SliderStyle.OutSet
      })
        .width('100%')
        .showTips(true)
        .blockColor('#42A5F5')
        .selectedColor('#42A5F5')
        .trackColor('#E0E0E0')
        .onChange((newValue: number) => {
          onChangeValue(Math.round(newValue));
          this.item.lastUpdateTime = new Date().toLocaleTimeString();
        })
    }
    .padding(12)
    .backgroundColor('#F8FAFD')
    .borderRadius(10)
  }
}

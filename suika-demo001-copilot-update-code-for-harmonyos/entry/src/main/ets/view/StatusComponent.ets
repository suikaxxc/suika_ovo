import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import {
  AlarmStatusArray,
  WifiStatusArray
} from "../common/constants/Constant";

@Component
export struct StatusComponent {
  @Prop
  item: AquaticTankViewModel;

  build() {
    Column() {
      // 标题栏
      Row() {
        Text("实时状态")
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor('#333333')
        Blank()
        Row() {
          Circle()
            .width(8)
            .height(8)
            .fill(this.item.wifiStatus === 1 ? '#4CAF50' : '#FF5722')
          Text(WifiStatusArray[this.item.wifiStatus])
            .fontSize(12)
            .fontColor(this.item.wifiStatus === 1 ? '#4CAF50' : '#FF5722')
            .margin({ left: 4 })
        }
      }
      .width('100%')
      .margin({ bottom: 12 })

      // 水位指示
      Row() {
        Image($r('app.media.water_level'))
          .width(24)
          .height(24)
        Column() {
          Row() {
            Text("水位")
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text(`${this.item.waterLevel}%`)
              .fontSize(16)
              .fontWeight(FontWeight.Medium)
              .fontColor(this.getWaterLevelColor())
          }
          .width('100%')

          Progress({ value: this.item.waterLevel, total: 100, type: ProgressType.Linear })
            .width('100%')
            .height(8)
            .color(this.getWaterLevelColor())
            .backgroundColor('#E0E0E0')
            .margin({ top: 4 })
        }
        .layoutWeight(1)
        .margin({ left: 8 })
      }
      .width('100%')
      .padding(8)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 8 })

      // 水温指示
      Row() {
        Image($r('app.media.temperature'))
          .width(24)
          .height(24)
        Column() {
          Row() {
            Text("水温")
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text(`${this.item.waterTemp}°C`)
              .fontSize(16)
              .fontWeight(FontWeight.Medium)
              .fontColor(this.getWaterTempColor())
          }
          .width('100%')

          Row() {
            Text(`${this.item.waterTempMin}°C`)
              .fontSize(10)
              .fontColor('#999999')
            Progress({ value: this.getTempProgress(), total: 100, type: ProgressType.Linear })
              .layoutWeight(1)
              .height(8)
              .color(this.getWaterTempColor())
              .backgroundColor('#E0E0E0')
              .margin({ left: 4, right: 4 })
            Text(`${this.item.waterTempMax}°C`)
              .fontSize(10)
              .fontColor('#999999')
          }
          .width('100%')
          .margin({ top: 4 })
        }
        .layoutWeight(1)
        .margin({ left: 8 })
      }
      .width('100%')
      .padding(8)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 8 })

      // 光照强度
      Row() {
        Image($r('app.media.light'))
          .width(24)
          .height(24)
        Column() {
          Row() {
            Text("光照")
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text(`${this.item.lightIntensity} lux`)
              .fontSize(16)
              .fontWeight(FontWeight.Medium)
              .fontColor(this.getLightColor())
          }
          .width('100%')

          Row() {
            Text(`阈值: ${this.item.lightThreshold} lux`)
              .fontSize(10)
              .fontColor('#999999')
            Blank()
            Text(this.item.lightIntensity < this.item.lightThreshold ? "需补光" : "充足")
              .fontSize(10)
              .fontColor(this.item.lightIntensity < this.item.lightThreshold ? '#FF9800' : '#4CAF50')
          }
          .width('100%')
          .margin({ top: 4 })
        }
        .layoutWeight(1)
        .margin({ left: 8 })
      }
      .width('100%')
      .padding(8)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 8 })

      // 水质TDS指示
      Row() {
        Image($r('app.media.water_level'))
          .width(24)
          .height(24)
        Column() {
          Row() {
            Text("水质")
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text(`${this.item.tdsValue} ppm`)
              .fontSize(16)
              .fontWeight(FontWeight.Medium)
              .fontColor(this.getTdsColor())
          }
          .width('100%')

          Row() {
            Text(`${this.item.tdsMin} ppm`)
              .fontSize(10)
              .fontColor('#999999')
            Progress({ value: this.getTdsProgress(), total: 100, type: ProgressType.Linear })
              .layoutWeight(1)
              .height(8)
              .color(this.getTdsColor())
              .backgroundColor('#E0E0E0')
              .margin({ left: 4, right: 4 })
            Text(`${this.item.tdsMax} ppm`)
              .fontSize(10)
              .fontColor('#999999')
          }
          .width('100%')
          .margin({ top: 4 })
        }
        .layoutWeight(1)
        .margin({ left: 8 })
      }
      .width('100%')
      .padding(8)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 8 })

      // 报警状态
      if (this.item.alarmStatus > 0) {
        Row() {
          Image($r('app.media.alarm'))
            .width(20)
            .height(20)
          Text(this.item.alarmMessage || AlarmStatusArray[this.item.alarmStatus])
            .fontSize(14)
            .fontColor('#FF5722')
            .margin({ left: 8 })
        }
        .width('100%')
        .padding(8)
        .backgroundColor('#FFEBEE')
        .borderRadius(8)
        .margin({ top: 8 })
      }
    }
    .padding(14)
    .width('94%')
    .borderRadius(14)
    .backgroundColor(Color.White)
    .margin({ left: '3%', right: '3%' })
    .shadow({ radius: 8, color: '#1A000000', offsetX: 0, offsetY: 2 })
  }

  getWaterLevelColor(): string {
    if (this.item.waterLevel < this.item.waterLevelMin) {
      return '#FF5722';
    } else if (this.item.waterLevel > this.item.waterLevelMax) {
      return '#FF9800';
    }
    return '#4CAF50';
  }

  getWaterTempColor(): string {
    if (this.item.waterTemp < this.item.waterTempMin) {
      return '#2196F3';
    } else if (this.item.waterTemp > this.item.waterTempMax) {
      return '#FF5722';
    }
    return '#4CAF50';
  }

  getTempProgress(): number {
    const range = this.item.waterTempMax - this.item.waterTempMin;
    if (range <= 0) return 50;
    const progress = ((this.item.waterTemp - this.item.waterTempMin + 5) / (range + 10)) * 100;
    return Math.max(0, Math.min(100, progress));
  }

  getLightColor(): string {
    return this.item.lightIntensity < this.item.lightThreshold ? '#FF9800' : '#4CAF50';
  }

  getTdsColor(): string {
    if (this.item.tdsValue < this.item.tdsMin) {
      return '#2196F3';
    } else if (this.item.tdsValue > this.item.tdsMax) {
      return '#FF5722';
    }
    return '#4CAF50';
  }

  getTdsProgress(): number {
    const range = this.item.tdsMax - this.item.tdsMin;
    if (range <= 0) return 50;
    // 使用偏移量使进度条在边界值时不会完全为0或100，提供更好的视觉效果
    const offset = 50; // 偏移量，避免进度条在最小值时完全为空
    const totalRange = 100; // 总偏移范围，与offset配合使用
    const progress = ((this.item.tdsValue - this.item.tdsMin + offset) / (range + totalRange)) * 100;
    return Math.max(0, Math.min(100, progress));
  }
}

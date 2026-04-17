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
      Row() {
        Text("实时状态")
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor('#243447')
        Blank()
        Row() {
          Image($r('app.media.turbidity'))
            .width(18)
            .height(18)
          Text('浊度传感器')
            .fontSize(11)
            .fontColor('#6B7280')
            .margin({ left: 4, right: 8 })
        }
        .padding({ left: 6, right: 2, top: 3, bottom: 3 })
        .borderRadius(12)
        .backgroundColor('#F4F7FB')
        Row() {
          Circle()
            .width(8)
            .height(8)
            .fill(this.item.wifiStatus === 1 ? '#2ECC71' : '#FF6B35')
          Text(WifiStatusArray[this.item.wifiStatus])
            .fontSize(12)
            .fontWeight(FontWeight.Medium)
            .fontColor(this.item.wifiStatus === 1 ? '#2ECC71' : '#FF6B35')
            .margin({ left: 6 })
        }
        .padding({ left: 8, right: 10, top: 4, bottom: 4 })
        .borderRadius(16)
        .backgroundColor('#F3F7FF')
      }
      .width('100%')
      .margin({ bottom: 10 })

      Row({ space: 8 }) {
        this.MetricCard($r('app.media.water_level'), '水位', `${this.item.waterLevel} mm`,
          `${this.item.waterLevelMin}-${this.item.waterLevelMax} mm`, this.getWaterLevelColor(), '#ECF8EE')
        this.MetricCard($r('app.media.temperature'), '水温', `${this.item.waterTemp}°C`,
          `${this.item.waterTempMin}-${this.item.waterTempMax}°C`, this.getWaterTempColor(), '#EEF5FF')
      }
      .width('100%')
      .margin({ bottom: 6 })

      Row({ space: 8 }) {
        this.MetricCard($r('app.media.light'), '光照', `${this.item.lightIntensity} lux`,
          this.item.lightIntensity < this.item.lightThreshold ? '需补光' : '光照充足', this.getLightColor(), '#FFF9EA')
        this.MetricCard($r('app.media.water_level'), '水质', `${this.item.tdsValue} ppm`,
          `${this.item.tdsMin}-${this.item.tdsMax} ppm`, this.getTdsColor(), '#F1F7FF')
      }
      .width('100%')
      .margin({ bottom: 6 })

      Row({ space: 8 }) {
        this.MetricCard($r('app.media.turbidity'), '浊度', `${this.item.turbidityValue} NTU`,
          this.item.turbidityValue >= 700 ? '建议换水' : '状态正常', this.getTurbidityColor(), '#F7F2FF')
        this.MetricCard($r('app.media.plant'), '更新时间', this.item.lastUpdateTime || '--:--:--',
          '实时同步', '#42A5F5', '#EEF7FF')
      }
      .width('100%')

      if (this.item.alarmStatus > 0) {
        Row() {
          Image($r('app.media.alarm'))
            .width(18)
            .height(18)
          Text(this.item.alarmMessage || AlarmStatusArray[this.item.alarmStatus])
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
            .fontColor('#D94A38')
            .margin({ left: 8 })
        }
        .width('100%')
        .padding({ left: 10, right: 10, top: 8, bottom: 8 })
        .borderRadius(10)
        .backgroundColor('#FFF0EE')
        .margin({ top: 8 })
      }
    }
    .padding(10)
    .width('94%')
    .borderRadius(16)
    .backgroundColor('#FFFFFF')
    .margin({ left: '3%', right: '3%' })
    .shadow({ radius: 10, color: '#18000000', offsetX: 0, offsetY: 3 })
  }

  @Builder
  MetricCard(icon: Resource, title: string, value: string, subText: string, valueColor: string, bgColor: string) {
    Column() {
      Row() {
        Image(icon)
          .width(18)
          .height(18)
        Text(title)
          .fontSize(12)
          .fontColor('#5F6B7A')
          .margin({ left: 6 })
      }
      .width('100%')

      Text(value)
        .fontSize(16)
        .fontWeight(FontWeight.Bold)
        .fontColor(valueColor)
        .margin({ top: 4 })
        .width('100%')

      Text(subText)
        .fontSize(10)
        .fontColor('#8E99A7')
        .margin({ top: 2 })
        .width('100%')
    }
    .layoutWeight(1)
    .padding({ left: 8, right: 8, top: 8, bottom: 8 })
    .borderRadius(12)
    .backgroundColor(bgColor)
  }

  getWaterLevelColor(): string {
    if (this.item.waterLevel < this.item.waterLevelMin) {
      return '#FF6B35';
    } else if (this.item.waterLevel > this.item.waterLevelMax) {
      return '#FF9800';
    }
    return '#2EAF51';
  }

  getWaterTempColor(): string {
    if (this.item.waterTemp < this.item.waterTempMin) {
      return '#2196F3';
    } else if (this.item.waterTemp > this.item.waterTempMax) {
      return '#FF6B35';
    }
    return '#2EAF51';
  }

  getLightColor(): string {
    return this.item.lightIntensity < this.item.lightThreshold ? '#F59A23' : '#2EAF51';
  }

  getTdsColor(): string {
    if (this.item.tdsValue < this.item.tdsMin) {
      return '#2196F3';
    } else if (this.item.tdsValue > this.item.tdsMax) {
      return '#FF6B35';
    }
    return '#2EAF51';
  }

  getTurbidityColor(): string {
    if (this.item.turbidityValue >= 700) {
      return '#FF5722';
    }
    if (this.item.turbidityValue >= 500) {
      return '#FF9800';
    }
    return '#2EAF51';
  }
}

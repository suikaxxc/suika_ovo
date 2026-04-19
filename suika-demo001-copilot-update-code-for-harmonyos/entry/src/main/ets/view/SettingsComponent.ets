import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import {
  PlantTypeArray, GrowthStageArray, PlantParamsConfig
} from "../common/constants/Constant";
import { promptAction } from '@kit.ArkUI';

export interface SettingsChangeData {
  plantTypeIndex?: number;
  waterTempMin?: number;
  waterTempMax?: number;
  waterLevelMin?: number;
  waterLevelMax?: number;
  lightThreshold?: number;
  lightDuration?: number;
  growthStage?: number;
}

@Component
export struct SettingsComponent {
  @Link
  item: AquaticTankViewModel;
  // 回调函数不能使用@Prop装饰器，直接作为普通属性传递
  onSettingsChange: (settings: SettingsChangeData) => void = () => {
  };

  // 使用@State变量存储推荐参数，确保UI能够正确更新
  @State recommendedWaterTempMin: number = 18;
  @State recommendedWaterTempMax: number = 28;
  @State recommendedWaterLevelMin: number = 18;  // 单位: mm (YW01传感器范围0-90mm)
  @State recommendedWaterLevelMax: number = 54;  // 单位: mm
  @State recommendedLightThreshold: number = 300;
  @State recommendedLightDuration: number = 8;
  @State plantDescription: string = "";

  // 组件即将显示时，初始化推荐参数
  aboutToAppear(): void {
    this.updateRecommendedParams();
  }

  // 根据植物品种和生长阶段更新推荐参数
  updateRecommendedParams(): void {
    const params = PlantParamsConfig[this.item.plantTypeIndex];
    if (!params) return;

    // 获取生长阶段调整系数：幼苗期0.8, 生长期1.0, 成熟期1.2
    let adjustment: number = 1.0;
    switch (this.item.growthStage) {
      case 0: adjustment = 0.8; break;  // 幼苗期
      case 1: adjustment = 1.0; break;  // 生长期
      case 2: adjustment = 1.2; break;  // 成熟期
      default: adjustment = 1.0;
    }

    // 更新推荐参数（水温不受生长阶段影响）
    this.recommendedWaterTempMin = params.waterTempMin;
    this.recommendedWaterTempMax = params.waterTempMax;

    // 水位(mm)、光照阈值、补光时长根据生长阶段调整
    // YW01传感器范围0-90mm
    this.recommendedWaterLevelMin = Math.round(params.waterLevelMin * adjustment);
    this.recommendedWaterLevelMax = Math.min(90, Math.round(params.waterLevelMax * adjustment));
    this.recommendedLightThreshold = Math.round(params.lightThreshold * adjustment);
    this.recommendedLightDuration = Math.round(params.lightDuration * adjustment);
    this.plantDescription = params.description;
  }

  build() {
    Scroll() {
      Column() {
        // 标题
        Row() {
          Image($r('app.media.plant'))
            .width(24)
            .height(24)
          Text("植物养护设置")
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
            .margin({ left: 8 })
        }
        .width('100%')
        .margin({ bottom: 16 })

      // 植物品种选择
      Row() {
        Text("植物品种")
          .fontSize(14)
          .fontColor('#666666')
          .width(80)
        Select(PlantTypeArray.map((item, index) => {
          return { value: item } as SelectOption
        }))
          .selected(this.item.plantTypeIndex)
          .value(PlantTypeArray[this.item.plantTypeIndex])
          .font({ size: 14 })
          .fontColor('#333333')
          .selectedOptionFont({ size: 14 })
          .optionFont({ size: 14 })
          .layoutWeight(1)
          .onSelect((index: number) => {
            this.item.plantTypeIndex = index;
            this.updateRecommendedParams();
            this.applyPlantParams(index);
          })
      }
      .width('100%')
      .padding(12)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 12 })

      // 生长阶段选择
      Row() {
        Text("生长阶段")
          .fontSize(14)
          .fontColor('#666666')
          .width(80)
        Select(GrowthStageArray.map((item, index) => {
          return { value: item } as SelectOption
        }))
          .selected(this.item.growthStage)
          .value(GrowthStageArray[this.item.growthStage])
          .font({ size: 14 })
          .fontColor('#333333')
          .selectedOptionFont({ size: 14 })
          .optionFont({ size: 14 })
          .layoutWeight(1)
          .onSelect((index: number) => {
            this.item.growthStage = index;
            this.updateRecommendedParams();
            this.onSettingsChange({ growthStage: index } as SettingsChangeData);
          })
      }
      .width('100%')
      .padding(12)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 12 })

      // 推荐养护参数显示（根据植物品种和生长阶段动态变化）
      Column({ space: 8 }) {
        Text("推荐养护参数")
          .fontSize(14)
          .fontWeight(FontWeight.Medium)
          .fontColor('#333333')
          .width('100%')

        Row() {
          // 水温范围
          Column() {
            Text("水温范围")
              .fontSize(11)
              .fontColor('#999999')
            Text(`${this.recommendedWaterTempMin}-${this.recommendedWaterTempMax}°C`)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .fontColor('#FF5722')
              .margin({ top: 2 })
          }
          .alignItems(HorizontalAlign.Center)
          .padding(8)

          // 水位范围 (单位: mm, YW01传感器范围0-90mm)
          Column() {
            Text("水位范围")
              .fontSize(11)
              .fontColor('#999999')
            Text(`${this.recommendedWaterLevelMin}-${this.recommendedWaterLevelMax}mm`)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .fontColor('#2196F3')
              .margin({ top: 2 })
          }
          .alignItems(HorizontalAlign.Center)
          .padding(8)
        }
        .width('100%')
        .justifyContent(FlexAlign.SpaceEvenly)

        Row() {
          // 光照阈值
          Column() {
            Text("光照阈值")
              .fontSize(11)
              .fontColor('#999999')
            Text(`${this.recommendedLightThreshold} lux`)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .fontColor('#FFC107')
              .margin({ top: 2 })
          }
          .alignItems(HorizontalAlign.Center)
          .padding(8)

          // 补光时长
          Column() {
            Text("补光时长")
              .fontSize(11)
              .fontColor('#999999')
            Text(`${this.recommendedLightDuration} h/天`)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .fontColor('#4CAF50')
              .margin({ top: 2 })
          }
          .alignItems(HorizontalAlign.Center)
          .padding(8)
        }
        .width('100%')
        .justifyContent(FlexAlign.SpaceEvenly)
      }
      .width('100%')
      .padding(12)
      .backgroundColor('#E3F2FD')
      .borderRadius(8)
      .margin({ bottom: 12 })

      // 植物描述
      Column() {
        Text(this.plantDescription || "请选择植物品种")
          .fontSize(12)
          .fontColor('#666666')
          .lineHeight(20)
          .width('100%')
      }
      .width('100%')
      .padding(12)
      .backgroundColor('#FFF3E0')
      .borderRadius(8)

      // 应用AI推荐参数按钮
      Button("应用AI推荐参数")
        .fontSize(14)
        .fontColor(Color.White)
        .backgroundColor('#42A5F5')
        .width('100%')
        .height(40)
        .borderRadius(20)
        .margin({ top: 16 })
        .onClick(() => {
          this.applyPlantParams(this.item.plantTypeIndex);
          promptAction.showToast({
            message: `已应用${PlantTypeArray[this.item.plantTypeIndex]}的最优养护参数`,
            duration: 2000
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

  applyPlantParams(index: number) {
    const params = PlantParamsConfig[index];
    if (params) {
      this.item.waterTempMin = params.waterTempMin;
      this.item.waterTempMax = params.waterTempMax;
      this.item.waterLevelMin = params.waterLevelMin;
      this.item.waterLevelMax = params.waterLevelMax;
      this.item.lightThreshold = params.lightThreshold;
      this.item.lightDuration = params.lightDuration;
      this.onSettingsChange({
        plantTypeIndex: index,
        waterTempMin: params.waterTempMin,
        waterTempMax: params.waterTempMax,
        waterLevelMin: params.waterLevelMin,
        waterLevelMax: params.waterLevelMax,
        lightThreshold: params.lightThreshold,
        lightDuration: params.lightDuration
      } as SettingsChangeData);
    }
  }
}

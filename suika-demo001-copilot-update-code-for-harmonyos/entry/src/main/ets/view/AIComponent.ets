import AquaticTankViewModel from "../viewmodel/AquaticTankViewModel";
import { PlantTypeArray, GrowthStageArray } from "../common/constants/Constant";
import ResultData from '../model/ResultData';
import HttpUtil from '../common/utils/HttpUtil';
import { promptAction } from '@kit.ArkUI';

@Component
export struct AIComponent {
  @Prop
  item: AquaticTankViewModel;

  @State
  resultString: string = '点击"AI养护建议"获取智能养护策略';

  @State
  isBusy: boolean = false;

  aiRequest() {
    // 判断是否在请求中
    if (this.isBusy) {
      promptAction.showToast({
        message: 'AI智能分析中...',
        duration: 2000
      });
      return;
    }

    // 判断当前数据是否有效
    if (!this.item) {
      promptAction.showToast({
        message: '设备数据无效，请检查连接',
        duration: 2000
      });
      return;
    }

    // 获取当前植物和生长阶段信息
    const plantName = PlantTypeArray[this.item.plantTypeIndex] || "未知植物";
    const growthStage = GrowthStageArray[this.item.growthStage] || "未知阶段";

    // 构建AI请求参数
    const prompt = `请针对水生植物"${plantName}"在"${growthStage}"提供养护建议。
当前环境参数：水温${this.item.waterTemp}°C，水位${this.item.waterLevel}%，光照${this.item.lightIntensity}lux。
目标参数：水温${this.item.waterTempMin}-${this.item.waterTempMax}°C，水位${this.item.waterLevelMin}-${this.item.waterLevelMax}%，光照阈值${this.item.lightThreshold}lux。
请分析当前环境是否适合该植物生长，并给出具体的调整建议和养护注意事项，内容控制在150字以内。`;

    // 改变状态
    this.isBusy = true;

    // AI分析请求
    HttpUtil.post<ResultData, string>('ai/analysis', prompt)
      .then((data: ResultData) => {
        // 成功后处理数据
        if (data?.success) {
          this.resultString = `${data.data}`;
        } else {
          this.resultString = '获取AI建议失败，请稍后重试';
        }
      })
      .catch(() => {
        this.resultString = '网络请求失败，请检查网络连接';
      })
      .finally(() => {
        // 最后更新状态
        this.isBusy = false;
      });
  }

  build() {
    Column() {
      // 标题栏
      Row() {
        Row() {
          Image($r('app.media.ai'))
            .width(28)
            .height(28)
          Text("AI智能养护")
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
            .margin({ left: 8 })
        }

        Blank()

        if (this.isBusy) {
          Row() {
            LoadingProgress()
              .width(24)
              .height(24)
              .color('#42A5F5')
            Text('分析中...')
              .fontSize(12)
              .fontColor('#42A5F5')
              .margin({ left: 4 })
          }
        } else {
          Button('AI养护建议')
            .fontSize(13)
            .fontColor(Color.White)
            .backgroundColor('#42A5F5')
            .height(32)
            .borderRadius(16)
            .onClick(() => {
              this.aiRequest();
            })
        }
      }
      .width('100%')
      .margin({ bottom: 12 })

      // 当前养护植物信息
      Row() {
        Text(`当前植物：${PlantTypeArray[this.item.plantTypeIndex]}`)
          .fontSize(12)
          .fontColor('#1976D2')
          .fontWeight(FontWeight.Medium)
        Blank()
        Text(`生长阶段：${GrowthStageArray[this.item.growthStage]}`)
          .fontSize(12)
          .fontColor('#1976D2')
          .fontWeight(FontWeight.Medium)
      }
      .width('100%')
      .padding(10)
      .backgroundColor('#E3F2FD')
      .borderRadius(8)
      .margin({ bottom: 10 })

      // AI建议内容 - 增大高度使内容更易阅读
      Scroll() {
        Text(this.resultString)
          .fontSize(14)
          .lineHeight(24)
          .fontColor('#333333')
          .width('100%')
      }
      .layoutWeight(1)
      .padding(14)
      .backgroundColor('#FAFAFA')
      .borderRadius(10)
      .scrollBar(BarState.Auto)
    }
    .padding(14)
    .width('94%')
    .height(320)
    .borderRadius(14)
    .backgroundColor(Color.White)
    .margin({ left: '3%', right: '3%' })
    .shadow({ radius: 8, color: '#1A000000', offsetX: 0, offsetY: 2 })
  }
}
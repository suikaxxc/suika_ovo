import { McLineChart, Options } from '@mcui/mccharts'
import EnvironmentViewModel from '../viewmodel/EnvironmentViewModel';

@Component
export struct SwiperComponent {
  @State tempArray: number[] = [11, 11, 15, 13, 12, 35, 10];
  @State humArray: number[] = [51, 41, 45, 68, 60, 50, 40];
  @State gasArray: number[] = [101, 211, 215, 213, 212, 135, 110];

  @State tempOption: Options = new Options({
    title: {
      show: true,
      text: '最高气温',
      right: 20,
      top: 30
    },
    color:['#008000'],
    xAxis: {
      data: ['周一', '周二', '周三', '周四', '周五', '周六', '周日']
    },
    yAxis: {
      name: '单位：度'
    },
    series: [
      {
        name: '最高气温',
        data: this.tempArray
      }
    ]
  })
  @State humOption: Options = new Options({
    title: {
      show: true,
      text: '最高湿度',
      right: 20,
      top: 30
    },
    color:['#008000'],
    xAxis: {
      data: ['周一', '周二', '周三', '周四', '周五', '周六', '周日']
    },
    yAxis: {
      name: '单位：%'
    },
    series: [
      {
        name: '最高湿度',
        data: this.humArray
      }
    ]
  })
  @State gasOption: Options = new Options({
    title: {
      show: true,
      text: '可燃气体',
      right: 20,
      top: 30
    },
    color:['#008000'],
    xAxis: {
      data: ['周一', '周二', '周三', '周四', '周五', '周六', '周日']
    },
    yAxis: {
      name: '单位：PPM'
    },
    series: [
      {
        name: '可燃气体浓度',
        data: this.gasArray
      }
    ]
  })

  updata(array: number[], value: number) {
    if(!array)
      return ;
    array.push(value)
    if (array.length > 7) {
      array.splice(0, 1)
    }
  }

  upOption(){
    this.tempOption.setVal({
      series: [
        {
          name: '最高气温',
          data: this.tempArray
        }
      ]
    })
    this.humOption.setVal({
      series: [
        {
          name: '最高湿度',
          data: this.humArray
        }
      ]
    })
    this.gasOption.setVal({
      series: [
        {
          name: '可燃气体浓度',
          data: this.gasArray
        }
      ]
    })
  }
  @Prop @Watch('onItemUpdated')
  item: EnvironmentViewModel = new EnvironmentViewModel();
  onItemUpdated() {
    this.updata(this.tempArray, this.item.temp)
    this.updata(this.humArray, this.item.hum)
    this.updata(this.gasArray, this.item.gas)
    this.upOption();
  }

  build() {
    Swiper() {
      McLineChart({
        options: this.tempOption
      })
        .height('30%')

      McLineChart({
        options: this.humOption
      }).height('30%')

      McLineChart({
        options: this.gasOption
      })
        .height('30%')
    }
    .loop(true)
    .autoPlay(true)
    .interval(3000)
    .backgroundColor(Color.White)
    .indicator( // 设置圆点导航点样式
      new DotIndicator()
        .itemWidth(10)
        .itemHeight(10)
        .selectedItemWidth(18)
        .selectedItemHeight(12)
        .color(Color.Gray)
        .selectedColor(Color.Green))
    .margin({ left: 10, right: 10 })
    .padding(10)
    .borderRadius(10)
    .width('94%')
  }
}
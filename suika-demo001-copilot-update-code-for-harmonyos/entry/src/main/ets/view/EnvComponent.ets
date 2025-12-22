import { StatusArray } from "../common/constants/Constant";
import EnvironmentViewModel from "../viewmodel/EnvironmentViewModel";
import { CellComponent } from "./CellComponent";

@Extend(Line)
function lineStyle(backgroundColor: Color) {
  .width('94%')
  .height(1)
  .backgroundColor(backgroundColor)
  .margin({ top: 4, bottom: 4 })
}

@Component
export struct EnvComponent {
  @Prop
  item: EnvironmentViewModel;

  build() {
    Column() {
      CellComponent({ title: "空气温度：", value: `${this.item.temp} 度`, color: this.getFontColor() })
      Line().lineStyle(this.getFontColor())
      CellComponent({ title: "空气湿度：", value: `${this.item.hum} %`, color: this.getFontColor() })
      Line().lineStyle(this.getFontColor())
      CellComponent({ title: "可燃气体：", value: `${this.item.gas} PPM`, color: this.getFontColor() })
      Line().lineStyle(this.getFontColor())
      CellComponent({ title: "安全级别：", value: `${StatusArray[this.item.stat]}`, color: this.getFontColor() })

    }
    .backgroundColor(this.getBackgroudColor())
    .margin({ left: 10, right: 10 })
    .padding(10)
    .borderRadius(10)
    .width('94%')
  }

  getBackgroudColor(): Color {
    switch (this.item.stat) {
      case 0:
        return Color.Green;
      case 1:
        return Color.Yellow;
      case 2:
        return Color.Red;
      default:
        return Color.Gray;
    }
  }

  getFontColor(): Color {
    switch (this.item.stat) {
      case 0:
        return Color.White;
      case 1:
        return Color.Red;
      case 2:
        return Color.White;
      default:
        return Color.White;
    }
  }
}
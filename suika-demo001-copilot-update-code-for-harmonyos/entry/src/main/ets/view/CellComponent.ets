@Component
export struct CellComponent {
  @Prop
  title: string; //标题
  @Prop
  value: string; //值
  @Prop color: Color; //颜色

  build() {
    Row() {
      Text(`${this.title}`)
        .fontSize(18)
        .fontWeight(500)
        .fontColor(this.color)
        .textAlign(TextAlign.Start)
        .width('50%')
        .lineHeight(30)
      Text(`${this.value}`)
        .fontSize(18)
        .fontWeight(500)
        .fontColor(this.color)
        .width('40%')
        .textAlign(TextAlign.End)
        .lineHeight(30)
    }
    .justifyContent(FlexAlign.SpaceBetween)
    .padding({ left: 10, right: 10 })
    .width('100%')
  }
}
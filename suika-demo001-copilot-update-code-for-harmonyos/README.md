# suika-001

## 浊度图标替换说明

当前“浊度”卡片在代码中使用的图标资源名是：
- `app.media.turbidity`

对应资源文件路径：
- `/home/runner/work/suika_ovo/suika_ovo/suika-demo001-copilot-update-code-for-harmonyos/entry/src/main/resources/base/media/turbidity.png`

对应代码位置：
- `/home/runner/work/suika_ovo/suika_ovo/suika-demo001-copilot-update-code-for-harmonyos/entry/src/main/ets/view/StatusComponent.ets`

### 方式一（最简单，直接替换现有文件）

1. 准备你的新图标（建议PNG、透明背景、方形）
2. 用新图标覆盖：
   - `entry/src/main/resources/base/media/turbidity.png`
3. 保持文件名不变，无需改代码

> 注意：当前仓库已新增 `turbidity.png`（初始内容为占位图），后续可直接替换为真实浊度传感器图标。

### 当前代码状态

当前代码已完成“浊度独立图标”接入，`StatusComponent.ets` 的浊度卡片使用：
- `Image($r('app.media.turbidity'))`

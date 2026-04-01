# suika-001

## 浊度图标替换说明

当前“浊度”卡片在代码中使用的图标资源名是：
- `app.media.water_level`

对应资源文件路径：
- `/home/runner/work/suika_ovo/suika_ovo/suika-demo001-copilot-update-code-for-harmonyos/entry/src/main/resources/base/media/water_level.png`

对应代码位置：
- `/home/runner/work/suika_ovo/suika_ovo/suika-demo001-copilot-update-code-for-harmonyos/entry/src/main/ets/view/StatusComponent.ets`

### 方式一（最简单，直接替换现有文件）

1. 准备你的新图标（建议PNG、透明背景、方形）
2. 用新图标覆盖：
   - `entry/src/main/resources/base/media/water_level.png`
3. 保持文件名不变，无需改代码

> 注意：这种方式会同时影响所有引用 `app.media.water_level` 的地方，不仅是浊度卡片。

### 方式二（推荐，给浊度单独图标）

1. 新增资源文件（示例）：
   - `entry/src/main/resources/base/media/turbidity.png`
2. 在 `StatusComponent.ets` 的浊度行把：
   - `Image($r('app.media.water_level'))`
   改成：
   - `Image($r('app.media.turbidity'))`
3. 重新编译运行即可仅替换浊度图标，不影响水位图标。

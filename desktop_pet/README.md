# Windows EXE 桌面宠物（人物模仿猴子爬行）

这个目录提供一个独立的桌面宠物实现：
- 从人物图片中提取透明人物素材（可去掉瓶子并自动补全遮挡区域）
- 在桌面创建多个“人物宠物”
- 宠物在屏幕范围爬行、跳动，并将窗口边缘与桌面边界作为障碍物互动

## 1) 安装依赖

建议在 Windows + Python 3.10/3.11 环境：

```bash
pip install -r requirements.txt
```

## 2) 准备人物素材（去背景、去瓶子、补全）

输入原图后，可用矩形或手工 mask 指定“瓶子”区域：

```bash
python prepare_sprite.py \
  --input ./input/person.jpg \
  --output ./assets/person_clean.png \
  --bottle-rect 520,420,140,220
```

如果你已手工画好 mask（白色表示需要删除区域）：

```bash
python prepare_sprite.py \
  --input ./input/person.jpg \
  --output ./assets/person_clean.png \
  --bottle-mask ./input/bottle_mask.png
```

## 3) 运行桌面宠物

```bash
python pet_app.py --image ./assets/person_clean.png --count 8 --size 180 --fps 24
```

参数说明：
- `--image`: 透明背景人物 PNG
- `--count`: 同时出现的宠物数量
- `--size`: 单个宠物缩放尺寸
- `--fps`: 动画更新帧率

## 4) 打包 EXE

先安装打包工具：

```bash
pip install pyinstaller
```

打包：

```bash
pyinstaller --noconfirm --onefile --windowed pet_app.py --name monkey_person_pet
```

生成的 exe 在 `dist/monkey_person_pet.exe`。

## 备注

- 窗口障碍识别依赖 `pywin32`，仅在 Windows 下有效。
- 人物“模仿猴子爬行”通过随机攀爬/碰撞反弹/跳跃行为模拟。
- 若补全部分不自然，可更精细地提供 `--bottle-mask` 来提升修复效果。

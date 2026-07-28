import argparse
from pathlib import Path
from typing import Optional, Tuple

import cv2
import numpy as np
from rembg import remove


def parse_rect(value: Optional[str]) -> Optional[Tuple[int, int, int, int]]:
    if not value:
        return None
    x, y, w, h = [int(v.strip()) for v in value.split(",")]
    return x, y, w, h


def build_bottle_mask(shape: Tuple[int, int], mask_path: Optional[str], rect: Optional[Tuple[int, int, int, int]]) -> np.ndarray:
    h, w = shape
    mask = np.zeros((h, w), dtype=np.uint8)
    if mask_path:
        raw = cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)
        if raw is None:
            raise ValueError(f"无法读取 mask 文件: {mask_path}")
        raw = cv2.resize(raw, (w, h), interpolation=cv2.INTER_NEAREST)
        mask[raw > 127] = 255
    if rect:
        x, y, rw, rh = rect
        x2 = max(0, min(w, x + rw))
        y2 = max(0, min(h, y + rh))
        x = max(0, min(w, x))
        y = max(0, min(h, y))
        mask[y:y2, x:x2] = 255
    return mask


def main():
    parser = argparse.ArgumentParser(description="提取人物并去除瓶子，补全遮挡区域")
    parser.add_argument("--input", required=True, help="原始图片路径")
    parser.add_argument("--output", required=True, help="输出 PNG 路径")
    parser.add_argument("--bottle-mask", help="瓶子区域 mask（白色区域表示要删除）")
    parser.add_argument("--bottle-rect", help="瓶子矩形区域：x,y,w,h")
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        removed_bg = remove(f.read())

    arr = np.frombuffer(removed_bg, dtype=np.uint8)
    cut_img = cv2.imdecode(arr, cv2.IMREAD_UNCHANGED)
    if cut_img is None:
        raise ValueError("背景去除失败，请检查输入图片")

    if cut_img.shape[2] == 3:
        cut_img = cv2.cvtColor(cut_img, cv2.COLOR_BGR2BGRA)
    h, w = cut_img.shape[:2]
    rect = parse_rect(args.bottle_rect)
    mask = build_bottle_mask((h, w), args.bottle_mask, rect)

    bgr = cut_img[:, :, :3]
    alpha = cut_img[:, :, 3]

    inpainted = cv2.inpaint(bgr, mask, 7, cv2.INPAINT_TELEA)
    alpha[mask > 0] = 255

    out = np.dstack([inpainted, alpha])
    ok, encoded = cv2.imencode(".png", out)
    if not ok:
        raise ValueError("编码输出 PNG 失败")
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_bytes(encoded.tobytes())
    print(f"已生成人物素材：{args.output}")


if __name__ == "__main__":
    main()

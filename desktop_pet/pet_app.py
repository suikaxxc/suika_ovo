import argparse
import random
import sys
import tkinter as tk
from dataclasses import dataclass
from typing import List, Tuple

from PIL import Image, ImageOps, ImageTk


Rect = Tuple[int, int, int, int]


def screen_size(root: tk.Tk) -> Tuple[int, int]:
    return root.winfo_screenwidth(), root.winfo_screenheight()


def intersects(a: Rect, b: Rect) -> bool:
    ax1, ay1, ax2, ay2 = a
    bx1, by1, bx2, by2 = b
    return ax1 < bx2 and ax2 > bx1 and ay1 < by2 and ay2 > by1


def visible_window_rects() -> List[Rect]:
    if sys.platform != "win32":
        return []
    import win32con
    import win32gui

    rects: List[Rect] = []

    def enum_handler(hwnd, _):
        if not win32gui.IsWindowVisible(hwnd):
            return
        if win32gui.IsIconic(hwnd):
            return
        if win32gui.GetWindowLong(hwnd, win32con.GWL_EXSTYLE) & win32con.WS_EX_TOOLWINDOW:
            return
        left, top, right, bottom = win32gui.GetWindowRect(hwnd)
        if right - left < 80 or bottom - top < 80:
            return
        rects.append((left, top, right, bottom))

    win32gui.EnumWindows(enum_handler, None)
    return rects


def collision_rects(root: tk.Tk) -> List[Rect]:
    width, height = screen_size(root)
    wall = 80
    return [
        (-wall, -wall, 0, height + wall),
        (width, -wall, width + wall, height + wall),
        (-wall, -wall, width + wall, 0),
        (-wall, height, width + wall, height + wall),
        *visible_window_rects(),
    ]


@dataclass
class PetState:
    x: float
    y: float
    vx: float
    vy: float
    climbing: int = 0


class Pet:
    def __init__(self, root: tk.Tk, frames: List[Image.Image], x: int, y: int):
        self.root = root
        self.frame_idx = 0
        self.frames = [ImageTk.PhotoImage(f) for f in frames]
        self.width, self.height = frames[0].size
        self.state = PetState(x=x, y=y, vx=random.choice([-2.2, 2.2]), vy=0.0)
        self.window = tk.Toplevel(root)
        self.window.overrideredirect(True)
        self.window.attributes("-topmost", True)
        self.window.configure(bg="magenta")
        self.window.wm_attributes("-transparentcolor", "magenta")
        self.label = tk.Label(self.window, image=self.frames[0], bg="magenta", bd=0, highlightthickness=0)
        self.label.pack()

    def bounds(self) -> Rect:
        return (
            int(self.state.x),
            int(self.state.y),
            int(self.state.x + self.width),
            int(self.state.y + self.height),
        )

    def place(self):
        self.window.geometry(f"{self.width}x{self.height}+{int(self.state.x)}+{int(self.state.y)}")
        self.frame_idx = (self.frame_idx + 1) % len(self.frames)
        self.label.configure(image=self.frames[self.frame_idx])

    def update(self, obstacles: List[Rect], screen_w: int, screen_h: int):
        s = self.state
        if s.climbing > 0:
            s.climbing -= 1
            s.vx = random.choice([-1.3, 1.3])
            s.vy = random.choice([-2.5, -1.9, 1.9, 2.5])
        else:
            s.vy += 0.25
            if random.random() < 0.01:
                s.vx = random.choice([-2.2, -1.7, 1.7, 2.2])
            if random.random() < 0.015:
                s.vy = random.choice([-3.2, -2.8, 2.4])

        next_x = s.x + s.vx
        next_y = s.y + s.vy
        next_rect = (int(next_x), int(next_y), int(next_x + self.width), int(next_y + self.height))

        for obstacle in obstacles:
            if not intersects(next_rect, obstacle):
                continue
            ox1, oy1, ox2, oy2 = obstacle
            overlap_left = next_rect[2] - ox1
            overlap_right = ox2 - next_rect[0]
            overlap_top = next_rect[3] - oy1
            overlap_bottom = oy2 - next_rect[1]
            min_overlap = min(overlap_left, overlap_right, overlap_top, overlap_bottom)
            if min_overlap == overlap_left:
                next_x = ox1 - self.width
                s.vx = -abs(s.vx)
                s.climbing = random.randint(8, 22)
            elif min_overlap == overlap_right:
                next_x = ox2
                s.vx = abs(s.vx)
                s.climbing = random.randint(8, 22)
            elif min_overlap == overlap_top:
                next_y = oy1 - self.height
                s.vy = -abs(s.vy) * 0.6
            else:
                next_y = oy2
                s.vy = abs(s.vy)
            next_rect = (int(next_x), int(next_y), int(next_x + self.width), int(next_y + self.height))

        next_x = max(0, min(next_x, screen_w - self.width))
        next_y = max(0, min(next_y, screen_h - self.height))
        s.x, s.y = next_x, next_y
        self.place()


def build_frames(image_path: str, size: int) -> List[Image.Image]:
    base = Image.open(image_path).convert("RGBA")
    base.thumbnail((size, size), Image.Resampling.LANCZOS)
    if base.width < size:
        canvas = Image.new("RGBA", (size, base.height), (0, 0, 0, 0))
        canvas.paste(base, ((size - base.width) // 2, 0), base)
        base = canvas
    mirrored = ImageOps.mirror(base)
    tilted_a = base.rotate(5, resample=Image.Resampling.BICUBIC, expand=True)
    tilted_b = mirrored.rotate(-5, resample=Image.Resampling.BICUBIC, expand=True)
    return [base, tilted_a, mirrored, tilted_b]


class PetApp:
    def __init__(self, image_path: str, count: int, size: int, fps: int):
        self.root = tk.Tk()
        self.root.withdraw()
        self.fps = max(10, fps)
        self.screen_w, self.screen_h = screen_size(self.root)
        frames = build_frames(image_path, size)
        self.pets: List[Pet] = []
        for _ in range(max(1, count)):
            x = random.randint(0, max(1, self.screen_w - frames[0].width))
            y = random.randint(0, max(1, self.screen_h - frames[0].height))
            self.pets.append(Pet(self.root, frames, x, y))

    def tick(self):
        obstacles = collision_rects(self.root)
        for pet in self.pets:
            pet.update(obstacles, self.screen_w, self.screen_h)
        self.root.after(1000 // self.fps, self.tick)

    def run(self):
        self.tick()
        self.root.mainloop()


def main():
    parser = argparse.ArgumentParser(description="桌面宠物：人物模仿猴子爬行")
    parser.add_argument("--image", required=True, help="透明背景人物 PNG 路径")
    parser.add_argument("--count", type=int, default=6, help="宠物数量")
    parser.add_argument("--size", type=int, default=180, help="单个宠物显示尺寸")
    parser.add_argument("--fps", type=int, default=24, help="动画帧率")
    args = parser.parse_args()

    app = PetApp(args.image, args.count, args.size, args.fps)
    app.run()


if __name__ == "__main__":
    main()

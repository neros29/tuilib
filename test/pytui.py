from typing import List
from logging import getLogger
import sys
import atexit
from typing import Callable, Optional
sys.path.append("build/")
from enum import Enum, auto

import tui

logger = getLogger(__name__)
levels = {
    10: "DEBUG",
    20: "INFO",
    30: "WARNING",
    40: "ERROR",
    50: "CRITICAL",
}
def log(level, msg):
    with open("logs/test-log", "a") as f:
        f.write(f"[{levels[level]}]{msg}\n")

tui.set_logger(log)
def cleanup_logging():
    tui.set_logger(None)

atexit.register(cleanup_logging)

class style(int):
    NONE =          0
    BOLD =          1 << 0
    DIM =           1 << 1
    ITALIC =        1 << 2
    UNDERLINE =     1 << 3
    BLINKING =      1 << 4
    INVERSE =       1 << 5
    HIDDEN =        1 << 6
    STRIKETHROUGH = 1 << 7

class Key(int):
    NONE = 0
    UP = 1
    DOWN = 2
    RIGHT = 3
    LEFT = 4
    HOME = 5
    END = 6
    INSERT = 7
    DELETE = 8
    PAGEUP = 9
    PAGEDOWN = 10
    F1 = 11
    F2 = 12
    F3 = 13
    F4 = 14
    F5 = 15
    F6 = 16
    F7 = 17
    F8 = 18
    F9 = 19
    F10 = 20
    F11 = 21
    F12 = 22
    K_0X20 = 32
    K_0X21 = 33
    K_0X22 = 34
    K_0X23 = 35
    K_0X24 = 36
    K_0X25 = 37
    K_0X26 = 38
    K_0X27 = 39
    K_0X28 = 40
    K_0X29 = 41
    K_0X2A = 42
    K_0X2B = 43
    K_0X2C = 44
    K_0X2D = 45
    K_0X2E = 46
    K_0X2F = 47
    K_0 = 48
    K_1 = 49
    K_2 = 50
    K_3 = 51
    K_4 = 52
    K_5 = 53
    K_6 = 54
    K_7 = 55
    K_8 = 56
    K_9 = 57
    K_0X3A = 58
    K_0X3B = 59
    K_0X3C = 60
    K_0X3D = 61
    K_0X3E = 62
    K_0X3F = 63
    K_0X40 = 64
    K_0X5B = 91
    K_0X5C = 92
    K_0X5D = 93
    K_0X5E = 94
    K_0X5F = 95
    K_0X60 = 96
    K_A = 97
    K_B = 98
    K_C = 99
    K_D = 100
    K_E = 101
    K_F = 102
    K_G = 103
    K_H = 104
    K_I = 105
    K_J = 106
    K_K = 107
    K_L = 108
    K_M = 109
    K_N = 110
    K_O = 111
    K_P = 112
    K_Q = 113
    K_R = 114
    K_S = 115
    K_T = 116
    K_U = 117
    K_V = 118
    K_W = 119
    K_X = 120
    K_Y = 121
    K_Z = 122
    K_0X7B = 123
    K_0X7C = 124
    K_0X7D = 125
    K_0X7E = 126

class modifers(int):
    none =      0
    shift =     0b1
    alt =       0b10
    ctrl =      0b100
    super =     0b1000
    hyper =     0b10000
    meta =      0b100000
    caps_lock = 0b1000000
    num_lock =  0b10000000

class Kind(int):
    Key = 0;
    Text = 1;

class Cell:
    def __init__(self, character: tui.Cell) -> None:
        self._char = character

    def set_bg(self, r: int, g: int, b: int, a: int = 255):
        return self._char.set_bg(r, g, b, a)

    def set_fg(self, r: int, g: int, b: int, a: int = 255):
        return self._char.set_fg(r, g, b, a)

    def set_ch(self, ch: str):
        return self._char.set_ch(ch)

    def set_style(self, style: style):
        return self._char.set_style(style)


class Surface:
    def __init__(self, surface: tui.Surface) -> None:
        self._surf = surface

    def fill_bg(self, r: int, g: int, b: int, a: int = 255):
        return self._surf.fill_bg(r, g, b, a)

    def fill_fg(self, r: int, g: int, b: int, a: int = 255):
        return self._surf.fill_fg(r, g, b, a)

    def fill_ch(self, ch: str):
        for i in range(self.size()[0] * self.size()[1]):
            self.__getitem__(i).set_ch(ch)

    def fill_style(self, style):
        self._surf.fill_style(style)
        
    def set_z(self, z: int):
        return self._surf.set_z(z)

    def get_z(self) -> int:
        return self._surf.get_z()

    def set_offset(self, x: int, y: int):
        return self._surf.set_offset(x, y)

    def extend_size(self, direction: str, amount: int):
        return self._surf.extend_size(direction, amount)

    def offset(self) -> List[int]:
        return self._surf.offset()

    def size(self) -> List[int]:
        return self._surf.size()

    def blit(self, surf):
        return self._surf.blit(surf.surf)

    def register_func(self, func: Callable):
        return self._surf.register_func(func)


    # def register_keys(self, keys: list[str]):
    #     return self._surf.register_keys(keys)
    #
    # def get_event(self, event: str):
    #     return self._surf.get_event(event)

    def __getitem__(self, index: int):
        return Cell(self._surf[index])

class Tui:
    def __init__(self) -> None:
        self._tui = tui.Tui()

    def append(self, size: list[int], offset: list[int], ch: str, z: int):
        return Surface(self._tui.append(size, offset, ch, z))

    def get_screen_size(self): 
        return self._tui.getScreenSize()

    def update(self, timeout_ms: int):
        return self._tui.update(timeout_ms)

def register_func_wrapper(func: Callable):
    def wrapper(*args, **kwargs):
        new_args = list(args)
        for i, arg in enumerate(new_args):
            if isinstance(arg, tui.Event):
                new_args[i] = Event(arg)
        new_kwargs = {}
        for key, value in kwargs.items():
            if isinstance(value, tui.Event):
                new_kwargs[key] = Event(value)
            else:
                new_kwargs[key] = value
        return func(*new_args, **new_kwargs)

    return wrapper

class _PyEvent:
    def __init__(self) -> None:
        self.kind: int = Kind.Text
        self.text: str = ""
        self.key: int = Key.NONE
        self.modifer: int = modifers.none

class Event:
    def __init__(self, event: Optional[tui.Event] = None) -> None:
        self._event = event if event else _PyEvent()
        self.kind = self._event.kind
        self.text = self._event.text
        self.key = self._event.key
        self.modifer = self._event.modifer
    def __eq__(self, value: object, /) -> bool:
        if isinstance(value, Event):
            return self.kind == value.kind and self.text == value.text and self.key == value.key and self.modifer == value.modifer
        return NotImplemented
class Label:
    def __init__ (self, root: Surface, text: str, offset: list[int]) -> None:
        self._label = tui.Label(root._surf, text, offset)
        self._text = text

    def update(self):
        self._label.update(self._text)

    def set_text(self, new_text):
        self._text = new_text
        self.update()

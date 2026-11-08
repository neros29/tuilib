from typing import List
from logging import getLogger
import sys
import atexit
from typing import Callable, Optional
sys.path.append("../build/")
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
    with open("logs", "a") as f:
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
    Up = 1
    Down = 2
    Left = 3
    Right = 4
    Home = 5
    End = 6
    Delete = 7
    Insert = 8
    PageUp = 9
    PageDown = 10
    Backspace = 11
    F1 = 12
    F2 = 13
    F3 = 14
    F4 = 15
    F5 = 16
    F6 = 17
    F7 = 19
    F8 = 20
    F9 = 21
    F10 = 22
    F11 = 23
    F12 = 24

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
       self.update()

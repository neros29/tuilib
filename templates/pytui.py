from typing import List
from logging import getLogger
import atexit

import tui

logger = getLogger(__name__)
tui.set_logger(logger.log)
def cleanup_logging():
    tui.set_logger(None)

atexit.register(cleanup_logging)
class Character:
    def __init__(self, character: tui.Character) -> None:
        self._char = character

    def set_bg(self, r: int, g: int, b: int, a: int = 255):
        return self._char.set_bg(r, g, b, a)

    def set_fg(self, r: int, g: int, b: int, a: int = 255):
        return self._char.set_fg(r, g, b, a)

    def set_ch(self, ch: str):
        return self._char.set_ch(ch)


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
        
    def set_z(self, z: int):
        return self._surf.set_z(z)

    def get_z(self) -> int:
        return self._surf.get_z()

    def set_offset(self, x: int, y: int):
        return self._surf.set_offset(x, y)

    def offset(self) -> List[int]:
        return self._surf.offset()

    def size(self) -> List[int]:
        return self._surf.size()

    def blit(self, surf):
        return self._surf.blit(surf.surf)

    def register_keys(self, keys: list[str]):
        return self._surf.register_keys(keys)

    def get_event(self, event: str):
        return self._surf.get_event(event)

    def extend_size(self, direction: str, amount: int):
        return self._surf.extend_size(direction, amount)

    def __getitem__(self, index: int):
        return Character(self._surf[index])

class Tui:
    def __init__(self) -> None:
        self._tui = tui.Tui()

    def append(self, size: list[int], offset: list[int], ch: str, z: int):
        return Surface(self._tui.append(size, offset, ch, z))

    def get_screen_size(self): 
        return self._tui.getScreenSize()

    def update_screen(self):
        return self._tui.update()


class Label:
    def __init__ (self, root: Surface, text: str, offset: list[int]) -> None:
        self._label = tui.Label(root._surf, text, offset)
        self._text = text

    def update(self):
        self._label.update(self._text)

    def set_text(self, new_text):
        self._text = new_text
        self.update()

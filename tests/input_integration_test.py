import pytui
import logging
import time
import subprocess
import os
from typing import Iterable
class Main:
    def __init__(self, events: list[tuple[list[str], list[pytui.Event]]]) -> None:
        self.tui = pytui.Tui()
        self.test_events = events
        self.results = [[None for event in test[1]] for test in events]
        self.surf = self.tui.append([50, 5], [10, 10], " ", 0)
        self.surf.fill_bg(255, 0, 0)
        self.surf.fill_fg(255, 255, 255)
        self.surf.fill_style(pytui.style.BOLD | pytui.style.UNDERLINE)
        self.surf.register_func(self.events)
        self.lab = pytui.Label(self.surf, "hello world", [0, 0])
        self.index = 0
        self.running = True
        self.wait = 0.5
        self.new = True

    @pytui.register_func_wrapper
    def events(self, system_event: pytui.Event):
        pytui.log(10, f"System Event: {system_event.key.name=} {system_event.kind.name=} {system_event.modifer=} {system_event.text=}")
        for i, event in enumerate(self.test_events[self.index][1]):
            if event == system_event and not self.results[self.index][i]:
                pytui.log(10, f"Event: {event.key.name=} {event.kind.name=} {event.modifer=} {event.text=} {self.index=} Matched System Event")
                self.results[self.index][i] = True
                if all(self.results[self.index]):
                    pytui.log(10, "Received all inputs.")
                    self.stop_test()
                return True
        return False

    def send_event(self, events: list[str]):
        for i in events:
            cmd = ['kitten', '@', 'send-text', '--match-tab', f'id:{os.getenv("KITTY_WINDOW_ID")}', i]
            subprocess.run(cmd); 
        sent = ', '.join([repr(e) for e in events])
        pytui.log(10, f"Sent [{sent}]")

    def stop_test(self):
        if self.index + 1 >= len(self.test_events):
            self.running = False

        self.index += 1
        self.new = True

    def run(self):
        self.lab.update()
        start = time.perf_counter()
        events = []
        while self.running:
            if events:
                self.send_event([events.pop()])

            if self.new:
                # pytui.log(10, f"Got all text in {time.perf_counter() - start} seconds")
                pytui.log(10, f"{'=' * 20} Starting Test {self.index} {'=' * 20}")
                events = self.test_events[self.index][0].copy()
                events.reverse()
                start = time.perf_counter()
                self.new = False
                self.tui.update(0);

            if time.perf_counter() > start + self.wait:
                pytui.log(10, "Input timed out.")
                self.stop_test()
            self.tui.update(16)
        return self.results

    def exit(self):
        self.tui.cleanup()

def make_event(e_dict: dict):
    e = pytui.Event()
    if e_dict["type"] == "key":
        e.kind = pytui.Kind.Key
        e.key = e_dict["key"]
    if e_dict["type"] == "paste":
        e.kind = pytui.Kind.Paste
        e.text = e_dict["text"]
    if e_dict["type"] == "text":
        e.text = e_dict["text"]
    e.modifer = e_dict.get('modifier', pytui.modifers.none)
    return e

events = []
events.append((['a'], [make_event({"type": "text", "text": "a"})]))
events.append((['A'], [make_event({"type": "text", "text": "A"})]))
events.append((['ab'], [make_event({"type": "text", "text": "a"}), make_event({"type": "text", "text": "b"})]))
events.append((['ø'], [make_event({"type": "text", "text": "ø"})]))
events.append((['e\u0301'], [make_event({"type": "text", "text": "e\u0301"})]))
events.append((['👍'], [make_event({"type": "text", "text": "👍"})]))
events.append((['👍'], [make_event({"type": "text", "text": "👍"})]))
events.append((['\x80'], []))
events.append((['\xff'], []))
events.append((['\r'], [make_event({"type": "key", "key": pytui.Key.ENTER})]))
events.append((['\t'], [make_event({"type": "key", "key": pytui.Key.TAB})]))
#
events.append((['\x08'], [make_event({"type": "key", "key": pytui.Key.BACKSPACE})]))
events.append((['\x7f'], [make_event({"type": "key", "key": pytui.Key.BACKSPACE})]))

# events.append((['\x00'], [make_event({"type": "key", "key": pytui.Key.SPACE, "modifier": pytui.modifers.ctrl})]))
events.append((['\x01'], [make_event({"type": "key", "key": pytui.Key.K_A, "modifier": pytui.modifers.ctrl})]))

events.append((['0', '1', '9'], [make_event({"type": "text", "text": "0"}), make_event({"type": "text", "text": "1"}),  make_event({"text": "9", "type": "text"})]))

# events.append((['\x1b'], [make_event({"type": "key", "key": pytui.Key.Escape})]))
events.append((['\x1b', 'a'], [make_event({"type": "key", "key": pytui.Key.K_A, "modifier": pytui.modifers.alt})]))
events.append((['\x1bx'], [make_event({"type": "key", "key": pytui.Key.K_X, "modifier": pytui.modifers.alt})]))
events.append((['\x1b\x1bOA'], [make_event({"type": "key", "key": pytui.Key.UP, "modifier": pytui.modifers.alt})]))
events.append((['\x1b\x7f'], [make_event({"type": "key", "key": pytui.Key.BACKSPACE, "modifier": pytui.modifers.alt})]))
events.append((['\x1b\r'], [make_event({"type": "key", "key": pytui.Key.ENTER, "modifier": pytui.modifers.alt})]))
events.append((['\x1bOA'], [make_event({"type": "key", "key": pytui.Key.UP})]))
events.append((['\x1b[A'], [make_event({"type": "key", "key": pytui.Key.UP})]))
events.append((['\x1bOB'], [make_event({"type": "key", "key": pytui.Key.DOWN})]))
events.append((['\x1b[B'], [make_event({"type": "key", "key": pytui.Key.DOWN})]))
events.append((['\x1bOC'], [make_event({"type": "key", "key": pytui.Key.RIGHT})]))
events.append((['\x1b[C'], [make_event({"type": "key", "key": pytui.Key.RIGHT})]))
events.append((['\x1bOD'], [make_event({"type": "key", "key": pytui.Key.LEFT})]))
events.append((['\x1b[D'], [make_event({"type": "key", "key": pytui.Key.LEFT})]))

events.append((['\x1b[A\x1b[A'], [make_event({"type": "key", "key": pytui.Key.UP}), make_event({"type": "key", "key": pytui.Key.UP})]))
events.append((['\x1b[', 'A'], [make_event({"type": "key", "key": pytui.Key.UP})]))
events.append((['\x1b', '[', 'A'], [make_event({"type": "key", "key": pytui.Key.UP})]))

events.append((['\x1b[7~'], [make_event({"type": "key", "key": pytui.Key.HOME})]))
events.append((['\x1b[1~'], [make_event({"type": "key", "key": pytui.Key.HOME})]))
events.append((['\x1bOH'], [make_event({"type": "key", "key": pytui.Key.HOME})]))
events.append((['\x1b[H'], [make_event({"type": "key", "key": pytui.Key.HOME})]))

events.append((['\x1b[8~'], [make_event({"type": "key", "key": pytui.Key.END})]))
events.append((['\x1b[4~'], [make_event({"type": "key", "key": pytui.Key.END})]))
events.append((['\x1bOF'], [make_event({"type": "key", "key": pytui.Key.END})]))
events.append((['\x1b[F'], [make_event({"type": "key", "key": pytui.Key.END})]))

events.append((['\x1b[6~'], [make_event({"type": "key", "key": pytui.Key.PAGEDOWN})]))
events.append((['\x1b[5~'], [make_event({"type": "key", "key": pytui.Key.PAGEUP})]))
events.append((['\x1b[3~'], [make_event({"type": "key", "key": pytui.Key.DELETE})]))
events.append((['\x1b[2~'], [make_event({"type": "key", "key": pytui.Key.INSERT})]))

events.append((['\x1b[24~'], [make_event({"type": "key", "key": pytui.Key.F12})]))
events.append((['\x1b[23~'], [make_event({"type": "key", "key": pytui.Key.F11})]))
events.append((['\x1b[21~'], [make_event({"type": "key", "key": pytui.Key.F10})]))
events.append((['\x1b[20~'], [make_event({"type": "key", "key": pytui.Key.F9})]))
events.append((['\x1b[19~'], [make_event({"type": "key", "key": pytui.Key.F8})]))
events.append((['\x1b[18~'], [make_event({"type": "key", "key": pytui.Key.F7})]))
events.append((['\x1b[17~'], [make_event({"type": "key", "key": pytui.Key.F6})]))
events.append((['\x1b[15~'], [make_event({"type": "key", "key": pytui.Key.F5})]))
events.append((['\x1bOS'], [make_event({"type": "key", "key": pytui.Key.F4})]))
events.append((['\x1bOR'], [make_event({"type": "key", "key": pytui.Key.F3})]))
events.append((['\x1bOQ'], [make_event({"type": "key", "key": pytui.Key.F2})]))
events.append((['\x1bOP'], [make_event({"type": "key", "key": pytui.Key.F1})]))

events.append((['\x1b[1;5C'],[make_event({"type": "key", "key": pytui.Key.RIGHT, "modifier": pytui.modifers.ctrl})]))
events.append((['\x1b[1;2A'],[make_event({"type": "key", "key": pytui.Key.UP, "modifier": pytui.modifers.shift})]))
events.append((['\x1b[1;3D'],[make_event({"type": "key", "key": pytui.Key.LEFT, "modifier": pytui.modifers.alt})]))
events.append((['\x1b[1;10A'],[make_event({"type": "key", "key": pytui.Key.UP, "modifier": (pytui.modifers.shift | pytui.modifers.super)})]))
events.append((['\x1b[3;5~'],[make_event({"type": "key", "key": pytui.Key.DELETE, "modifier": pytui.modifers.ctrl})]))
events.append((['\x1b[15;2~'],[make_event({"type": "key", "key": pytui.Key.F5, "modifier": pytui.modifers.shift})]))
events.append((['\x1b[Z'], [make_event({"type": "key", "key": pytui.Key.TAB, "modifier": pytui.modifers.shift})]))

events.append((['\x1b[Z', 'xyz'], [make_event({"type": "key", "key": pytui.Key.TAB, "modifier": pytui.modifers.shift}), make_event({"type": "text", "text": "x"}), make_event({"type": "text", "text": "y"}), make_event({"type": "text", "text": "z"})]))
events.append((['\x1b[99z', 'a'], [make_event({"type": "text", "text": "a"})]))
# events.append((['\x00a'], [make_event({"type": "key", "modifier": pytui.modifers.ctrl, "key": pytui.Key.SPACE}), make_event({"type": "text", "text": "a"})]))

# events.append((['\x1b[I'] / '\x1b[O' , Focus in / Focus out ))
# events.append((['\x1b[<0;10;5M', '\x1b[<0;10;5m'] , Mouse press then release at (10,5) ))
events.append((['\x1b[200~hi\x1b[201~'] , [make_event({"type": "paste", "text": "hi"})]))
events.append((['\x1b[200~a\x1b[Ab\x1b[201~'] , [make_event({"type": "paste", "text": "a\x1b[Ab"})]))
# events.append((['\x1b[12;40R'] , Cursor position report row 12, col 40 (not a key) ))
# events.append((['\x1b]0;title\x07'] , skipped (not a key); following input still works ))
# events.append((['\x1b]0;title\x1b\\'] , skipped (ST-terminated) ))
events.append((['\x1b[99999999999z'] , []))
events.append((['\xF0'] , []))
events.append((['\xE2\xA2', 'a'] , []))
events.append((['\x1bO', 'P'] , []))
events.append((['\x1b\xC3\xB8'] , []))
events.append((['\x1b[1;5P'], [make_event({"type": "key", "key": pytui.Key.F1, "modifer": pytui.modifers.ctrl})]))
# events.append((['\x1b['] + 300 digits , abandoned after the length cap ))
# events.append((['\x1b['] then silence 300 ms , pending sequence discarded ))
# events.append((['\x1bOp'] (keypad 0) , ignored or mapped, but not a jam ))
events.append((['\n'], [make_event({"type": "key", "key": pytui.Key.K_J, "modifier": pytui.modifers.ctrl})]))

print("\x1b[?1049h", end="", flush=True) # enter alt
pytui.log(20, "=" * 10 + "PROGRAM STARTED" + "=" * 10)

main = Main(events)
results = main.run()
main.exit()

print("\x1b[?1049l", end="", flush=True) # exit alt
total = 0
for i, (event, result) in enumerate(zip(events, results)):
    end_results = True
    print(f"Test {i}")
    if len(result) == 0:
        print(f"\t✅Passed Event. With no output. Input was {event[0]}")
    for i, r in enumerate(result):
        end_results = end_results and r
        e = event[1][i]
        if r:
            print(f"\t✅Passed Event. {e.key.name=} {e.kind.name=} {e.modifer=} {e.text=}. Key sent: '{event[0]}'")
        else:
            print(f"\t❌Failed Event. {e.key.name=} {e.kind.name=} {e.modifer=} {e.text=}. Key sent: '{event[0]}'")
    if end_results:
        print()
        total += 1;
print(f"Passed {total}/{len(events)} {(total / len(events)) * 100}%")

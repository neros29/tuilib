import pathlib

MAIN_KEYS = "data/keys.def"
UNIQUE_KEYS = "data/unique_keys.def"

pathlib.Path(MAIN_KEYS).parent.mkdir(exist_ok=True)
pathlib.Path(UNIQUE_KEYS).parent.mkdir(exist_ok=True)

def clear():
    with open(MAIN_KEYS, "w") as f:
        f.write("// AUTO GENERATED FILE\n")
        f.write("// AUTHOR templates/generate_keys.py\n")

    with open(UNIQUE_KEYS, "w") as f:
        f.write("// AUTO GENERATED FILE\n")
        f.write("// AUTHOR templates/generate_keys.py\n")


def to_str(s):
    return f"\"{s}\""

def write_keys(keys: list[dict]):
    have_ids = []
    used_ids = []
    need_ids = []
    for ch in keys:
        if ch.get("id") is not None:
            used_ids.append(ch.get("id"))
            have_ids.append(ch)
        else:
            need_ids.append(ch)
    for key in need_ids:
        done = False
        for i in have_ids:
            if i['name'] == key['name']:
                key['id'] = i['id']
                have_ids.append(key)
                done = True
                break
        if done:
            continue
        index = 0;
        while True:
            if index not in used_ids:
                key["id"] = index
                have_ids.append(key)
                used_ids.append(index);
                break
            index += 1;
    have_ids.sort(key=lambda x: int(x["id"]))
    for key in have_ids:
        creat_key(key)
    ks = {}
    for key in have_ids:
        ks[key['name']] = key['id']
    for k, v in ks.items():
        with open(UNIQUE_KEYS, "a") as f:
            f.write(f"U_KEY({k.upper()}, {int(v)}, \"{k.upper()}\")\n")

def creat_key(key: dict):
    with open(MAIN_KEYS, "a") as f:
        f.write(f'KEY({key["name"].upper()}, {int(key["id"])}, "{key["type"].upper()}", {int(key.get("parameter", "1"))}, \'{key.get("end_byte", "~")}\')\n')

clear()
keys = []
import string
printable = string.printable.replace('\r', '').replace('\n', '').replace('\t', '').replace('\x0b', '').replace('\x0c', '').replace(' ', '')
keys.append({"name": 'SPACE', 'id': ord(' '), "type": 'ASCII'})
for char in printable:
    char = char.lower()
    ascii_val = ord(char)
    if char.isalnum():
        name = f"K_{char}"
    else:
        name = f"K_0x{ascii_val:02X}"
    keys.append({"name": name, "id": ord(char), "type": 'ASCII'})

def add_csi(name: str, param: int, end_byte: str):
    keys.append({'name': name, 'type': 'CSI', 'parameter': param, 'end_byte': end_byte})

def add_ss3(name, end_byte):
    keys.append({'name': name, 'type': 'SS3', 'end_byte': end_byte})

def add_ss3_and_csi(name: str, param: int, end_byte: str):
    add_csi(name, param, end_byte)
    add_ss3(name, end_byte)

keys.append({"name": 'NONE', 'id': 0, 'type': 'NONE'})
add_ss3_and_csi('UP', 1, 'A')
add_ss3_and_csi('DOWN', 1, 'B')
add_ss3_and_csi('RIGHT', 1, 'C')
add_ss3_and_csi('LEFT', 1, 'D')

# | Home | `CSI H`, `SS3 H` (xterm and most modern terminals); `CSI 1 ~` (Linux console, screen, tmux); `CSI 7 ~` (rxvt) |
add_ss3_and_csi('HOME', 1, 'H')
add_csi('HOME', 1,'~')
add_csi('HOME', 7,'~')

# | End | `CSI F`, `SS3 F` (xterm and most modern); `CSI 4 ~` (Linux console, screen, tmux); `CSI 8 ~` (rxvt) |
add_ss3_and_csi('END', 1, 'F')
add_csi('END', 4,'~')
add_csi('END', 8,'~')

# | Insert | `CSI 2 ~` |
add_csi('INSERT', 2, '~')
# | Delete | `CSI 3 ~` |
add_csi('DELETE', 3, '~')
# | PageUp | `CSI 5 ~` |
add_csi('PAGEUP', 5, '~')
# | PageDown | `CSI 6 ~` |
add_csi('PAGEDOWN', 6, '~')

# | F1-F4 | `SS3 P` `SS3 Q` `SS3 R` `SS3 S` | `CSI 11 ~` ... `CSI 14 ~` | `CSI [ A` ... `CSI [ D` |
for i in range(1, 5):
    add_ss3(f'F{i}', chr(ord('P') + (i - 1)))
    add_csi(f'F{i}', 11 + (i - 1), '~')
    # add_csi(f'F{i}', 1, chr(ord('A') + (i - 1))) 
# | F5 | `CSI 15 ~` | `CSI 15 ~` | `CSI [ E` |
add_csi('F5', 15, '~')
add_csi('F5', 1, 'E')
# | F6-F10 | `CSI 17 ~` `18` `19` `20` `21` | same | `CSI 17 ~` ... `CSI 21 ~` |
for i in range(6, 11):
    add_csi(f'F{i}', (17 - 6) + i, '~')
# | F11, F12 | `CSI 23 ~`, `CSI 24 ~` | same | same |
for i in range(11, 13):
    add_csi(f'F{i}', (23 - 11) + i, '~')
# | F13-F20 | `CSI 25 ~` `26` `28` `29` `31` `32` `33` `34` (VT220 style) | same | same |

write_keys(keys)


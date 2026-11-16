import pathlib

MAIN_KEYS = "data/keys.def"
UNIQUE_KEYS = "data/unique_keys.def"
TYPE_ENUM = "data/key_types.def"

pathlib.Path(MAIN_KEYS).parent.mkdir(exist_ok=True)
pathlib.Path(UNIQUE_KEYS).parent.mkdir(exist_ok=True)
pathlib.Path(TYPE_ENUM).parent.mkdir(exist_ok=True)

def clear():
    with open(MAIN_KEYS, "w") as f:
        f.write("// AUTO GENERATED FILE\n")
        f.write("// AUTHOR templates/generate_keys.py\n")

    with open(UNIQUE_KEYS, "w") as f:
        f.write("// AUTO GENERATED FILE\n")
        f.write("// AUTHOR templates/generate_keys.py\n")

    with open(TYPE_ENUM, "w") as f:
        f.write("// AUTO GENERATED FILE\n")
        f.write("// AUTHOR templates/generate_keys.py\n")

TYPES = {
        "NONE": 0,
        "ASCII": 1,
        "CSI": 2,
        "SS3": 3,
        "CTRL": 4,
        }

def creat_types():
    with open(TYPE_ENUM, "a") as f:
        for k, v in TYPES.items():
            f.write(f"TYPE({k}, {v})\n")

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
        if key['name'].upper() not in ks:
            ks[key['name'].upper()] = key['id']
    for k, v in ks.items():
        with open(UNIQUE_KEYS, "a") as f:
            f.write(f"U_KEY({k.upper()}, {int(v)}, \"{k.upper()}\")\n")

def creat_key(key: dict):
    with open(MAIN_KEYS, "a") as f:
        end_byte = ord(key.get('end_byte', "~"))
        f.write(f'KEY({key["name"].upper()}, {int(key["id"])}, {TYPES[key["type"].upper()]}, {int(key.get("parameter", "0"))}, {end_byte})\n')

clear()
creat_types()
keys = []
import string
printable = string.printable.replace('\r', '').replace('\n', '').replace('\t', '').replace('\x0b', '').replace('\x0c', '').replace(' ', '')
keys.append({"name": 'SPACE', 'id': ord(' '), "type": 'ASCII'})
for char in printable:
    char = char.lower()
    ascii_val = ord(char)
    if char.isalnum():
        name = f"K_{char.upper()}"
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

used_CO = []
def add_ctrl(name, CO_byte):
    if CO_byte not in used_CO:
        used_CO.append(CO_byte)
        keys.append({'name': name, 'type': 'CTRL', 'parameter': 4, 'end_byte': CO_byte})

def add_key(name, CO_byte):
    if CO_byte not in used_CO:
        used_CO.append(CO_byte)
        keys.append({'name': name, 'type': 'CTRL', 'end_byte': CO_byte})


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

# ========== CONTROL KEYS ===============
# Dec Hex Name  Usual key
#   0 00  NUL   Ctrl+Space, Ctrl+@, Ctrl+2
#   1 01  SOH   Ctrl+A
#   2 02        Ctrl+B
#   3 03  ETX   Ctrl+C
#   4 04  EOT   Ctrl+D
#   5 05        Ctrl+E
#   6 06        Ctrl+F
#   7 07  BEL   Ctrl+G
#   8 08  BS    Ctrl+H  (also Backspace on some terminals)
#   9 09  HT    Tab, Ctrl+I
#  10 0A  LF    Ctrl+J  (also Enter if ICRNL is on)
#  11 0B        Ctrl+K
#  12 0C        Ctrl+L
#  13 0D  CR    Enter, Ctrl+M
#  14 0E        Ctrl+N
#  15 0F        Ctrl+O
#  16 10        Ctrl+P
#  17 11  DC1   Ctrl+Q  (XON, swallowed if IXON on)
#  18 12        Ctrl+R
#  19 13  DC3   Ctrl+S  (XOFF, swallowed if IXON on)
#  20 14        Ctrl+T
#  21 15        Ctrl+U
#  22 16        Ctrl+V
#  23 17        Ctrl+W
#  24 18  CAN   Ctrl+X
#  25 19        Ctrl+Y
#  26 1A  SUB   Ctrl+Z
#  27 1B  ESC   Escape, Ctrl+[, Ctrl+3
#  28 1C        Ctrl+\, Ctrl+4
#  29 1D        Ctrl+], Ctrl+5
#  30 1E        Ctrl+^, Ctrl+6
#  31 1F        Ctrl+_, Ctrl+/ (on many terminals), Ctrl+7
# 127 7F  DEL   Backspace (the usual default), Ctrl+?, Ctrl+8

add_key('BACKSPACE', chr(0x7f))
add_key('BACKSPACE', chr(0x08))
add_key('TAB', chr(0x09))
add_key('ENTER', chr(0x0A))
add_key('ENTER', chr(0x0D))

add_ctrl("SPACE", chr(0x0))
for ch in "abcdefghijklmnopqrstuvwxyz":
    add_ctrl(f'K_{ch.upper()}', chr(ord(ch) & 0x1F))
for num, ch in enumerate("4567"):
    add_ctrl(f'K_{ch.upper()}', chr(num + 0x1C))

write_keys(keys)

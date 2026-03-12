from typing import Tuple, List

if __name__ == "__main__":
    file_path = "../assets/dragon_ansi.txt"
    output_path = "../assets/dragon-ansi"

    blueprint_lines: List[str]
    max_len: int = 0

    with open(file_path, 'rt') as file:
        blueprint_lines = file.read().split("\n")

    for line in blueprint_lines:
        if len(line) > max_len:
            max_len = len(line)

    while blueprint_lines[-1] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(-1)
    while blueprint_lines[0] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(0)

    with open(f"{output_path}.blprt", 'wt') as file:
        file.write(f"{max_len}\n")
        for line in blueprint_lines:
            file.write(f"{line: <{max_len}}\n")

    print(max_len, len(blueprint_lines), max_len * len(blueprint_lines))

    with open(f"{output_path}.clr", 'wb') as file:
        file.write(b"\x07"*(max_len*len(blueprint_lines))) # default colour is 0x07, normal terminal grey

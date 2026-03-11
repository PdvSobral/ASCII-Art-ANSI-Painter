from typing import Tuple, List

ANSI4 = {
    0: '\033[30m',  # 0: Black
    1: '\033[31m',  # 1: Red
    2: '\033[32m',  # 2: Green
    3: '\033[33m',  # 3: Yellow
    4: '\033[34m',  # 4: Blue
    5: '\033[35m',  # 5: Magenta
    6: '\033[36m',  # 6: Cyan
    7: '\033[37m',  # 7: White
    8: '\033[90m',  # 8: Bright Black (Gray)
    9: '\033[91m',  # 9: Bright Red
    10: '\033[92m', # 10: Bright Green
    11: '\033[93m', # 11: Bright Yellow
    12: '\033[94m', # 12: Bright Blue
    13: '\033[95m', # 13: Bright Magenta
    14: '\033[96m', # 14: Bright Cyan
    15: '\033[97m', # 15: Bright White
}


def read_blueprint(file_path) -> Tuple[int, List[str]]:
    with open(file_path, 'rt') as file:
        # Size of line and blueprint
        return int(file.readline()), file.read().split("\n")

def read_colors(file_path) -> bytes:
    with open(file_path, 'rb') as file:
        return file.read()

def assemble_art(blueprint: List[str], colors: bytes, line_size: int):
    ansi_art = ""

    for i, v in enumerate(blueprint):
        for j, char in enumerate(v):
            ansi_art += f"\033[38;5;{colors[i*line_size+j]}m{char}" # Convert to ANSI8 escape code
        ansi_art += '\n\033[0m' # Reset color after each line

    return ansi_art

if __name__ == "__main__":
    # TODO: add mode ANSI4
    size, blueprint = read_blueprint('../example/example.blprt')
    colors = read_colors('../example/example.clr')
    ansi_art = assemble_art(blueprint, colors, size)
    print(ansi_art)

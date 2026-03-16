from typing import List
from sys import argv, exit as exit_

OUTPUT_NAME: str = "out.txt"
PROJECT_NAME: str = ""
OPTIMIZE: bool = False


def print_help():
    print(f"Usage: python3 {argv[0]} [OPTIONS...] [FLAGS...] input_project_name")
    print("OPTIONS:")
    print("\t<-h, --help> : Show this help message.")
    print("\t<-o, --output> output_name : Define custom output file name. All chars must be respect the [azAZ.-] regex.")
    print("FLAGS:")
    print("\t<-x, --optimize> : Instead of giving each character in the final blueprint an ANSI escape sequence, reuse the previous colour if possible")
    #print("\t<-m, --mode> ansi4|ansi8 : Select color mode (default ansi8)")

def assemble_art(blueprint: List[str], colors: bytes, line_size: int, optimize: bool = False):
    # TODO: make optimize mode have a specific flag for optimizing
    #  only by line (current) or file wide (check colour continuation between lines)
    ansi_art: str = ""
    char_color: int = 0x00 # although not necessary, initialize
    for line_num, str_line in enumerate(blueprint):
        for column_num, char in enumerate(str_line):
            if (optimize is False) or (column_num == 0) or (char_color != colors[line_num*line_size+column_num]):
                char_color = colors[line_num*line_size+column_num]
                ansi_art += f"\033[38;5;{colors[line_num * line_size + column_num]}m" # Convert to ANSI8 escape code

            ansi_art += char

        ansi_art += '\033[0m\n' # FIXME: Reset color after each line, will later be an option, a flag
    return ansi_art

def is_valid_project_name(string: str) -> bool | int:
    for ch in string:
        ch = ord(ch)
        #         a            z         A            Z         0             9            -            .
        if not ((0x61 <= ch <= 0x7A) or (0x41 <= ch <= 0x5A) or (0x30 <= ch <= 0x39) or ch == 0x2D or ch == 0x2E):
            return ch
    return True


if __name__ == "__main__":
    # TODO: add ansi4 mode
    # TODO: add optimize flag
    if ("-h" in argv) or ("--help" in argv):
        print_help()
        exit_(0)

    while len(argv) > 1:
        #if argv[1] in ("-m", "--mode"):
        #    if len(argv) > 2:
        #        MODE = argv[2].lower()
        #        if MODE not in ("ansi4", "ansi8"):
        #            print("Invalid mode! Use ansi4 or ansi8.")
        #            exit_(1)
        #        argv.pop(1)
        #        argv.pop(1)
        #    else:
        #        print("No mode provided!")
        #        exit_(1)
        if argv[1] in ("-o", "--output"):
            if len(argv) > 2:
                OUTPUT_NAME = argv[2]
                c: bool | int = is_valid_project_name(OUTPUT_NAME)
                if c is not True:
                    print(f"Invalid character '{chr(c)}' found in `output_name`!")
                    exit_(1)
            else:
                print(f"No output project name provided!")
                exit_(1)
            argv.pop(1)
            argv.pop(1)
        elif argv[1] in ("-x", "--optimize"):
            OPTIMIZE = True
            argv.pop(1)
        elif len(argv) == 2:
            PROJECT_NAME = argv[1]
            c: bool | int = is_valid_project_name(PROJECT_NAME)
            if c is not True:
                print(f"Invalid character '{chr(c)}' found in `input_project_name`!")
                exit_(1)
            argv.pop(1)
        else:
            print(f"Unrecognized option '{argv[1]}!'\nUse '--help' to see usage.")
            exit_(1)

    if PROJECT_NAME == "":
        print("No poject name to load provided!")
        exit_(1)

    size: int
    blueprint_lines: List[str]
    with open(f"{PROJECT_NAME}.blprt", 'rt') as file:
        size, blueprint_lines = int(file.readline()), file.read().split("\n")

    while blueprint_lines[-1] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(-1)
    while blueprint_lines[0] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(0)

    colors: bytes
    with open(f"{PROJECT_NAME}.clr", 'rb') as file:
        colors = file.read()

    if len(colors) < size * len(blueprint_lines):
        print("Colours file size does not match blueprint dimensions!")
        exit_(1)

    ansi_art = assemble_art(blueprint_lines, colors, size, OPTIMIZE)

    with open(OUTPUT_NAME, "wt") as file:
        file.write(ansi_art)

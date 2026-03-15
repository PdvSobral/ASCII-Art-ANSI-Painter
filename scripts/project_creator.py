from typing import List
from sys import argv, exit as exit_

OUTPUT_NAME: str = "out"
INPUT_NAME: str = ""

def print_help():
    print(f"Usage: python3 {argv[0]} [OPTIONS...] input_file")
    print("input_file : File to turn into project")
    print("OPTIONS:")
    print("\t<-h, --help> : Show the program help/usage message.")
    print("\t<-o, --output> output_name : Define custom output project name. All chars must be respect the [azAZ.-] regex.")

if __name__ == "__main__":

    if ("-h" in argv) or ("--help" in argv):
        print_help()
        exit_(0)

    while len(argv) > 1: # always has the program name
        if argv[1] == "-o" or argv[1] == "--output":
            if len(argv) > 2:
                OUTPUT_NAME = argv[2]
                for c in OUTPUT_NAME:
                    c = ord(c)
                    #         a            z         A            Z         0             9            -            .
                    if not ((0x61 <= c <= 0x7A) or (0x41 <= c <= 0x5A) or (0x30 <= c <= 0x39) or c == 0x2D or c == 0x2E):
                        print(f"Invalid character '{chr(c)}' found in <art_file>!")
                        exit_(1)
            else:
                print(f"No output project name provided!")
                exit_(1)
            argv.pop(1)
            argv.pop(1)
        elif len(argv) == 2:
            INPUT_NAME = argv[1]
            argv.pop(1)
        else:
            print(f"Unrecognized option '{argv[1]}!'\nUse '--help' to see usage.")
            exit_(1)

    if INPUT_NAME == "":
        print("No input file provided!")
        exit_(1)

    blueprint_lines: List[str]
    max_len: int = 0

    with open(INPUT_NAME, "rt") as file:
        blueprint_lines = file.read().split("\n")

    for line in blueprint_lines:
        if len(line) > max_len:
            max_len = len(line)

    while blueprint_lines[-1] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(-1)
    while blueprint_lines[0] == "" or blueprint_lines[-1] is None:
        blueprint_lines.pop(0)

    with open(f"{OUTPUT_NAME}.blprt", "wt") as file:
        file.write(f"{max_len}\n")
        for line in blueprint_lines:
            file.write(f"{line: <{max_len}}\n")

    print(max_len, len(blueprint_lines), max_len * len(blueprint_lines))

    with open(f"{OUTPUT_NAME}.clr", "wb") as file:
        # default colour is 0x07, normal terminal grey
        file.write(b"\x07"*(max_len*len(blueprint_lines)))

    exit_(0)

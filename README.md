# ANCII Art ANSI Painter

ANCII Art ANSI Painter (very creative, I know) is a terminal-friendly ASCII art painter designed for both ANSI4 and ANSI8 color modes. For now, only ANSI8 mode is fully functional.  

It provides a TUI (Text User Interface) to paint ASCII art, complementing existing ASCII art creation and editing projects rather than being a standalone replacement tool.

## Features

- Terminal emulator and TTY friendly (tested mostly on TTY).
- Paint existing ASCII art.
- Support for ANSI4 and ANSI8 color modes (ANSI8 is fully working).
- Simple keybinds.
- Portable usage via AppImage.

## Build/Installation

To build the program from source, first clone the repo and then run:

```bash
bash ./build/build.sh
```

This will:

- Compile the binary using `gcc`.
- Generate an AppImage in the /build directory for portable usage.

> The usage is identical for the binary and the AppImage. The AppImage exists for portability in case a compiled binary doesn’t run due to missing dependencies or ELF issues.

> NOTE: if using the binary, make sure that all .so files present in the build directory are in the same directory as the binary, or the binary will fail to find the custom shared libraries

## Usage

1. Navigate to the directory containing your ASCII art project files (.blprt and .clr formats).  
   > Currently, these must be created manually from the ASCII art to paint.

2. Run the binary or AppImage:
    ```bash
    $ ./build/ANCIIArtAnsiPainter.AppImage
    $ # OR
    $ ./build/ANCIIArtAnsiPainter
    ```

3. Select the mode (ANSI4 or ANSI8) from the menu.  
   > For now, only ANSI8 works.

4. Use the keybinds displayed at the bottom left of the interface to paint your art.

## Project creation
Example of project creation:

```bash
$ cd ./assets
$ python3 ../scripts/project_creator.py -o dragon-ansi dragon_ansi
```

## Turning the Project into an ASCII ANSI text file
Example of project "compilation":

```bash
$ cd ./assets
$ python3 ../scripts/art_assembler.py -o dragon-ansi.txt -x dragon-ansi
```

## Current Limitations

- Project creation and exportation (compilation) is currently done with supplementary python scripts
- Project creation is opinionated; the program only supports conversion to `.blprt` or `.clr` files of simple ASCII art.
- Only ANSI8 mode is functional; ANSI4 mode is not yet implemented.

## TODO

- Implement ANSI4 support.
- Add built-in project creation.
- Add built-in project exporting.
- Improve error handling and file validation.

## Background

ANCII Art ANSI Painter started as a personal project to create a custom ASCII art for my terminal (originally for Neofetch, now FastFetch). After not finding a tool that fit my needs, I built this one. Some people have shown interest, so it’s now available for anyone who wants to use it.
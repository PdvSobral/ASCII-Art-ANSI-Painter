#!/bin/bash
# stops script on first error
set -e

APP_NAME="ASCII-ANSI-PAINTER"
VERSION="v0.0.1b"

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

SRC="$PROJECT_ROOT/src"
SCRIPT="$PROJECT_ROOT/scripts"
BUILD="$PROJECT_ROOT/build"
APPIMAGE_DIR="$PROJECT_ROOT/packaging/AppDir"

mkdir -p "$BUILD"

# linkedlists is lookes into "$SRC/libraries" and appends lib<NAME>.so
gcc "$SRC/main.c"\
  -Wall -Wextra -Wpedantic\
  -L "$SRC/libraries"\
  -l linkedlists\
  -Wl,-rpath,'$ORIGIN'\
  -o "$BUILD/$APP_NAME-$VERSION-x86_64"

cp "$SRC/libraries/"*.so "$BUILD/"

# changed file name to main, as expected by appimage previous configurations
cp "$BUILD/$APP_NAME-$VERSION-x86_64" "$APPIMAGE_DIR/usr/bin/main"
cp "$SRC/libraries/"*.so "$APPIMAGE_DIR/usr/bin/"

"$SCRIPT/appimagetool-x86_64.AppImage" "$APPIMAGE_DIR" "$BUILD/$APP_NAME-$VERSION-x86_64.AppImage"

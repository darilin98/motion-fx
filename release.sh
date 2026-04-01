#!/bin/bash

BUNDLE="$1"
BINARY="$BUNDLE/Contents/MacOS/motion-fx"
FRAMEWORKS="$BUNDLE/Contents/Frameworks"

mkdir -p "$FRAMEWORKS"

dylibbundler -od -b \
  -x "$BINARY" \
  -d "$FRAMEWORKS" \
  -p @loader_path/../Frameworks/

xattr -cr "$BUNDLE"
codesign --force --sign - "$FRAMEWORKS"/*.dylib
codesign --force --sign - "$BUNDLE"

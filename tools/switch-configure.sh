#!/bin/bash

./configure \
    --disable-debug \
    --with-platform=switch \
    --host=aarch64-none-elf \
    --with-toolchain="${DEVKITPRO}/devkitA64" \
    --prefix="${HOME}/switch-tools/kodi-deps" \
    --with-tarballs="${HOME}/switch-tools/kodi-tarballs"

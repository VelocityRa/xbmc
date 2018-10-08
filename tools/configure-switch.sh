#!/bin/bash

source /etc/profile.d/devkit-env.sh

./configure \
    --disable-debug \
    --with-platform=switch \
    --host=aarch64-none-elf \
    --with-toolchain="${DEVKITPRO}/devkitA64" \
    --prefix="${HOME}/switch-tools/kodi-deps" \
    --with-tarballs="${HOME}/switch-tools/kodi-tarballs"

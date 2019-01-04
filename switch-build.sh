#!/bin/bash

SWITCH_BUILD_DIR=kodi-switch-build

rm -rf $SWITCH_BUILD_DIR
mkdir $SWITCH_BUILD_DIR
pushd $SWITCH_BUILD_DIR

source switch-set-vars.sourcefile

# CMAKE_EXECUTABLE="../kodi/tools/depends/native/cmake-native/x86_64-linux-native/bin/cmake"
CMAKE_EXECUTABLE="/home/velo/switch-tools/kodi-deps/x86_64-linux-gnu-native/bin/cmake"

${CMAKE_EXECUTABLE} -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=/programming/kodi/tools/depends/target/Toolchain.cmake \
                          -DWITH_FFMPEG=ON \
                          -DENABLE_INTERNAL_FFMPEG=OFF \
                          -DFFMPEG_PATH=${DEVKITPRO}/portlibs/switch \
                          -DENABLE_AIRTUNES=OFF \
                          -DENABLE_PYTHON=ON \
                          -DENABLE_X=OFF \
                          -DENABLE_UPNP=OFF \
                          -DENABLE_OPTICAL=OFF \
                          -DVERBOSE=1 \
                          -DCMAKE_RULE_MESSAGES:BOOL=OFF \
                          -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
                          ../kodi
popd

# make -C $SWITCH_BUILD_DIR

# Not sure about these
#                          -DFFMPEG_LINK_EXECUTABLE=1 \
#                          -DENABLE_INTERNAL_RapidJSON=ON \

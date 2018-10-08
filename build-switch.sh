rm -rf kodi-switch-build
mkdir kodi-switch-build
pushd kodi-switch-build

source /etc/profile.d/devkit-env.sh
source $DEVKITPRO/switchvars.sh

CMAKE_EXECUTABLE="../kodi/tools/depends/native/cmake-native/x86_64-linux-native/bin/cmake"

${CMAKE_EXECUTABLE} -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=/programming/kodi/tools/depends/target/Toolchain.cmake \
                          -DWITH_FFMPEG=ON \
                          -DENABLE_INTERNAL_FFMPEG=OFF \
                          -DFFMPEG_PATH=${DEVKITPRO}/portlibs/switch \
                          -DENABLE_PYTHON=OFF \
                          -DENABLE_AIRTUNES=OFF \
                          -DENABLE_X=OFF \
                          -DENABLE_UPNP=OFF \
                          -DENABLE_OPTICAL=OFF \
                          -DVERBOSE=TRUE \
                          ../kodi
popd

# Not sure about these
#                          -DFFMPEG_LINK_EXECUTABLE=1 \
#                          -DENABLE_INTERNAL_RapidJSON=ON \

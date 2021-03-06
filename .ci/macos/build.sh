#!/bin/bash -ex

set -o pipefail

export Qt5_DIR=$PWD/qt@5/5.15.2
export SDL2DIR=$PWD/sdl2/2.0.14_1
export PATH="/usr/local/opt/ccache/libexec:/usr/local/opt/llvm/bin:$PATH"
# ccache configurations
export CCACHE_CPP2=yes
export CCACHE_SLOPPINESS=time_macros

# export CC="ccache AppleClang"
# export CXX="ccache AppleClang"
# export LDFLAGS="-L/usr/local/opt/llvm/lib"
# export CPPFLAGS="-I/usr/local/opt/llvm/include"

ccache -s

mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_PROCESSOR="arm64" -DENABLE_QT=0 -DCMAKE_OSX_ARCHITECTURES="arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" -DCMAKE_BUILD_TYPE=Release -DENABLE_QT_TRANSLATION=ON -DCITRA_ENABLE_COMPATIBILITY_REPORTING=${ENABLE_COMPATIBILITY_REPORTING:-"OFF"} -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DUSE_DISCORD_PRESENCE=ON -DENABLE_FFMPEG_AUDIO_DECODER=OFF -GNinja
ninja

ccache -s

#ctest -VV -C Release

#!/bin/sh -ex

brew update
brew unlink python@2 || true
rm '/usr/local/bin/2to3' || true
brew options sdl2
brew install qt5 sdl2 p7zip ccache ffmpeg llvm ninja || true
pip3 install macpack
wget https://homebrew.bintray.com/bottles/qt%405-5.15.2.arm64_big_sur.bottle.tar.gz
tar -xzf qt@5-5.15.2.arm64_big_sur.bottle.tar.gz



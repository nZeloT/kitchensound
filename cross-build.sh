#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -d $DIR/cmake-build-cross-release ]; then
	mkdir $DIR/cmake-build-cross-release
fi
cd $DIR/cmake-build-cross-release

export RASPBIAN_ROOTFS=$HOME/rpi/rootfs
export PATH=/opt/cross-pi-gcc/bin:/opt/cross-pi-gcc/libexec/gcc/arm-linux-gnueabihf/8.3.0:$PATH
export RASPBERRY_VERSION=1

cmake -G "CodeBlocks - Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$DIR/Toolchain-rpi.cmake -DCMAKE_BUILD_TYPE=Release $DIR
make -j 8

if [ -f $DIR/cmake-build-cross-release/kitchensound ]; then
	echo "Transfering succesful build"
	scp $DIR/cmake-build-cross-release/kitchensound pi@192.168.2.115:/home/pi/kitchensound/cmake-build-cross-release/kitchensound
fi

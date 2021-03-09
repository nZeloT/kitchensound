#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -d $DIR/cmake-cross-build ]; then
	mkdir $DIR/cmake-cross-build
fi
cd $DIR/cmake-cross-build

export RASPBIAN_ROOTFS=$HOME/rpi/rootfs
export PATH=/opt/cross-pi-gcc/bin:/opt/cross-pi-gcc/libexec/gcc/arm-linux-gnueabihf/8.3.0:$PATH
export RASPBERRY_VERSION=1

cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=$DIR/Toolchain-rpi.cmake -DCMAKE_BUILD_TYPE=Release $DIR
ninja -j 8

if [ -f $DIR/cmake-cross-build/kitchensound ]; then
	echo "Transfering succesful build"
	scp $DIR/cmake-cross-build/kitchensound pi@192.168.2.115:/home/pi/kitchensound/cross-build/kitchensound
fi

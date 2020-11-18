#! /bin/bash

set -x

SRC_DIR=$PWD
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=/var/tmp/build/some_perf/$BUILD_TYPE

if [[ "$1" == "clean" ]]
then
    echo Removing $BUILD_DIR
    rm -rf $BUILD_DIR
    exit 0
fi

echo Building $SRC_DIR into $BUILD_DIR...

mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE $SRC_DIR

if ! make; then
    echo "To have all dependencies installed"
fi

echo Build finished, check $BUILD_DIR

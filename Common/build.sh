#!/bin/bash

CMAKE=$(which cmake)
NINJA=$(which ninja)

mkdir -p cmake-build-release
$CMAKE -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S . -DCMAKE_CXX_COMPILER=g++ -B cmake-build-release

$CMAKE --build cmake-build-release --target clean -j 4
$CMAKE --build cmake-build-release --target all -j 4
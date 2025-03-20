#!/bin/bash

# GCC
mkdir build-gcc
echo "
****************
* GCC & Makefile
****************
"

cmake -G "Unix Makefiles" -B./build-gcc -S. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
    -DSPARGEL_LINUX_IS_DESKTOP=OFF -DSPARGEL_GPU_ENABLE_VULKAN=OFF

cmake --build ./build-gcc --parallel $(nproc)

cd build-gcc && ctest && cd ..

# Clang
mkdir build-clang
echo "
****************
* Clang & Ninja
****************
"

cmake -G Ninja -B./build-clang -S. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DSPARGEL_LINUX_IS_DESKTOP=OFF -DSPARGEL_GPU_ENABLE_VULKAN=OFF

cmake --build ./build-clang --parallel $(nproc)

cd build-clang && ctest && cd ..

# base
FROM ubuntu:latest

# workdir
WORKDIR /app

# basic toolchain
RUN apt-get update && apt-get install -y \
    make \
    g++

# utils
RUN apt-get install -y \
    wget

# CMake
RUN mkdir tmp  \
    && wget https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-linux-x86_64.sh -O ./tmp/install-cmake.sh \
    && chmod +x ./tmp/install-cmake.sh \
    && ./tmp/install-cmake.sh --skip-license --prefix=/usr/local \
    && rm -r tmp

# dependencies
RUN apt-get install -y \
    libunwind-dev \
    libx11-xcb-dev

# source code
COPY . .

# cmake config
RUN mkdir build \
    && cmake -B./build -S. -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
        -DSPARGEL_GPU_ENABLE_VULKAN=OFF -DSPARGEL_ENABLE_OPENGL=OFF

# build
RUN cmake --build ./build --parallel $(nproc)

# test
RUN cd build \
    && ctest \
    && cd ..

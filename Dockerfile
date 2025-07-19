# base
FROM ubuntu:latest

# workdir
WORKDIR /app

# toolchain and utils
RUN apt-get update

RUN apt-get install -y wget \
    && mkdir tmp \
    && wget https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-linux-x86_64.sh -O ./tmp/install-cmake.sh \
    && chmod +x ./tmp/install-cmake.sh \
    && ./tmp/install-cmake.sh --skip-license --prefix=/usr/local \
    && rm -r tmp

RUN apt-get install -y \
    clang g++ \
    make ninja-build

RUN apt-get install -y \
    git python3

# dependencies
RUN apt-get install -y \
    libunwind-dev \
    libharfbuzz-dev libfreetype-dev

# source code
COPY . .

CMD ["bash", "./docker-build.sh"]

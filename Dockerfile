FROM runtimeverificationinc/ubuntu:bionic

RUN apt-get update &&        \
    apt-get install -y       \
    binutils-dev             \
    bison                    \
    clang-8                  \
    cmake                    \
    flex                     \
    g++                      \
    git                      \
    gperf                    \
    libboost-all-dev         \
    libcap-dev               \
    libcrypto++-dev          \
    libdouble-conversion-dev \
    libevent-dev             \
    libgflags-dev            \
    libgmp-dev               \
    libgoogle-glog-dev       \
    libiberty-dev            \
    libjemalloc-dev          \
    libkrb5-dev              \
    liblz4-dev               \
    liblzma-dev              \
    libmpfr-dev              \
    libnuma-dev              \
    libprocps-dev            \
    libprotobuf-dev          \
    libsasl2-dev             \
    libsecp256k1-dev         \
    libsnappy-dev            \
    libsodium-dev            \
    libssl-dev               \
    libtool                  \
    libyaml-dev              \
    libzstd-dev              \
    make                     \
    pkg-config               \
    protobuf-compiler        \
    rapidjson-dev            \
    sudo                     \
    unzip                    \
    wget                     \
    zlib1g-dev

USER user:user

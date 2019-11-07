FROM ubuntu:bionic

RUN apt-get update && \
    apt-get install -y \
    cmake \
    clang-8 \
    zlib1g-dev \
    bison \
    flex \
    libgmp-dev \
    libmpfr-dev \
    libyaml-dev \
    libjemalloc-dev \
    pkg-config \
    protobuf-compiler \
    libprotobuf-dev \
    libcrypto++-dev \
    libsecp256k1-dev \
    rapidjson-dev \
    libssl-dev \
    libprocps-dev \
    git \
    g++ \
    libgflags-dev \
    libgoogle-glog-dev \
    libkrb5-dev \
    libsasl2-dev \
    libnuma-dev \
    libcap-dev \
    gperf \
    libevent-dev \
    libtool \
    libboost-all-dev \
    libsnappy-dev \
    wget \
    unzip \
    libiberty-dev \
    liblz4-dev \
    liblzma-dev \
    make \
    binutils-dev \
    libsodium-dev \
    libzstd-dev \
    libdouble-conversion-dev

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN groupadd -g $GROUP_ID user && \
    useradd -m -u $USER_ID -s /bin/sh -g user user

USER $USER_ID:$GROUP_ID

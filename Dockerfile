FROM ubuntu:bionic

RUN apt-get update && \
    apt-get install -y git cmake clang-8 zlib1g-dev bison flex libboost-test-dev libgmp-dev libmpfr-dev libyaml-dev libjemalloc-dev curl pkg-config protobuf-compiler libprotobuf-dev libcrypto++-dev libsecp256k1-dev

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN groupadd -g $GROUP_ID user && \
    useradd -m -u $USER_ID -s /bin/sh -g user user

USER $USER_ID:$GROUP_ID

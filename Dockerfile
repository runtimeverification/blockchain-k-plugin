FROM ubuntu:bionic

RUN apt-get update && \
    apt-get install -y git cmake clang-8 llvm-8-tools lld-8 zlib1g-dev bison flex libboost-test-dev libgmp-dev libmpfr-dev libyaml-dev libjemalloc-dev curl maven pkg-config protobuf-compiler libprotobuf-dev

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN groupadd -g $GROUP_ID user && \
    useradd -m -u $USER_ID -s /bin/sh -g user user

USER $USER_ID:$GROUP_ID

from ubuntu:bionic

run apt-get update && \
    apt-get install -y git cmake clang-8 llvm-8-tools lld-8 zlib1g-dev bison flex libboost-test-dev libgmp-dev libmpfr-dev libyaml-dev libjemalloc-dev curl maven pkg-config protobuf-compiler

arg user_id=1000
arg group_id=1000
run groupadd -g $group_id user && \
    useradd -m -u $user_id -s /bin/sh -g user user

user $user_id:$group_id

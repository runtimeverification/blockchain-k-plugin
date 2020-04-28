FROM runtimeverificationinc/ubuntu:bionic

RUN    apt-get update                \
    && apt-get upgrade --yes         \
    && apt-get install --yes         \
            bison                    \
            clang-8                  \
            cmake                    \
            flex                     \
            git                      \
            libboost-test-dev        \
            libgflags-dev            \
            libgmp-dev               \
            libjemalloc-dev          \
            libmpfr-dev              \
            libprocps-dev            \
            libprotobuf-dev          \
            libsecp256k1-dev         \
            libssl-dev               \
            libyaml-dev              \
            pkg-config               \
            protobuf-compiler        \
            rapidjson-dev            \
            sudo                     \
            zlib1g-dev

# install more recent version of crypto++ because Debian derivatives package
# libcrypto++ version 5.6.4 incorrectly so that the blake2 hash function can be
# compiled but not invoked at link time
RUN    curl "http://http.us.debian.org/debian/pool/main/libc/libcrypto++/libcrypto++8_8.2.0-2_amd64.deb"    > libcpp.deb    \
    && curl "http://http.us.debian.org/debian/pool/main/libc/libcrypto++/libcrypto++-dev_8.2.0-2_amd64.deb" > libcppdev.deb \
    && dpkg -i libcpp.deb libcppdev.deb                                                                                     \
    && rm libcpp.deb libcppdev.deb

USER user:user

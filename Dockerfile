ARG K_COMMIT
FROM runtimeverificationinc/kframework-k:ubuntu-bionic-${K_COMMIT}

RUN    apt-get update                \
    && apt-get upgrade --yes         \
    && apt-get install --yes         \
            clang-8                  \
            cmake                    \
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
            pkg-config               \
            protobuf-compiler        \
            rapidjson-dev            \
            zlib1g-dev

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN groupadd -g $GROUP_ID user && useradd -m -u $USER_ID -s /bin/sh -g user user

USER user:user
WORKDIR /home/user

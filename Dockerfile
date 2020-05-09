ARG K_COMMIT
FROM runtimeverificationinc/kframework-k:ubuntu-bionic-${K_COMMIT}

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

USER user:user

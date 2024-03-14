ARG BASE_IMAGE
ARG K_VERSION
ARG Z3_VERSION
FROM runtimeverificationinc/ubuntu-${BASE_IMAGE}-z3:${Z3_VERSION} as Z3

ARG BASE_IMAGE
ARG K_VERSION
FROM runtimeverificationinc/kframework-k:ubuntu-${BASE_IMAGE}-${K_VERSION}

COPY --from=Z3 /usr/bin/z3 /usr/bin/z3

ARG LLVM_VERSION

RUN    apt-get update             \
    && apt-get upgrade --yes      \
    && apt-get install --yes      \
            autoconf              \
            cargo                 \
            clang-${LLVM_VERSION} \
            cmake                 \
            curl                  \
            libprocps-dev         \
            libsecp256k1-dev      \
            libssl-dev            \
            libtool

ARG USER=github-user
ARG GROUP=${USER}
ARG USER_ID=1000
ARG GROUP_ID=${USER_ID}
RUN groupadd -g ${GROUP_ID} ${GROUP} && useradd -m -u ${USER_ID} -s /bin/sh -g ${GROUP} ${USER}

USER ${USER}:${GROUP}

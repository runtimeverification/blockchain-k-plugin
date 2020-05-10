# Blockchain K Plugin

This plugin is used by K semantics that wish to incorporate blockchain and/or
cryptographic operations. Typically, developers and end users will never need
to directly clone or build this plugin---instead, it is imported into other
projects and built as part of their build process.

The various features of this plugin depend on several cryptographic libraries.
To simplify life for plugin users, this Makefile includes logic to install its
cryptographic library dependencies in case the packaged version is obsolete
(e.g. `cryptopp` on Debian based distributions) or non-existent (e.g.
`cpp-httplib`). If being invoked from a super-project, you can call this
project's Makefile as follows:

`make PREFIX=/path/to/install/prefix <library>`

to build `<library>` and install it at the given prefix. We currently provide:

- `cpp-httplib`
- `libff`
- `libcryptopp`
- `libsecp256k1`

## Build Instructions

If you want to manually build this plugin for testing or development purposes,
you may use the following instructions.

### Install Depdendencies

The blockchain plugin for the K framework requires the K framework as well as a set of external libraries.

#### Install the K Framework

See the K framework website and [build it from source](https://github.com/kframework/k/) or
install a [pre-built release binary](https://github.com/kframework/k/releases).

#### Install Other Dependencies

##### Package Manager Assisted Installation

Ubuntu:

```
sudo apt-get install git bison flex cmake clang libboost-test-dev libcrypto++-dev libgflags-dev libgmp-dev libjemalloc-dev libmpfr-dev libprocps-dev libprotobuf-dev libsecp256k1-dev libssl-dev libyaml-dev pkg-config protobuf-compiler rapidjson-dev zlib1g-dev
```

Arch:

```
sudo pacman -S git bison flex cmake clang boost crypto++ gflags gmp jemalloc mpfr procps-ng protobuf libsecp256k1 openssl libyaml pkgconf rapidjson zlib
```

OSX (Brew):

```
brew install git bison flex cmake llvm boost cryptopp gflags gmp jemalloc mpfr protobuf openssl libyaml pkg-config rapidjson zlib
```

The dependency `libsecp256k1` is not included in brew core, but can be installed via brew using a slightly modified version of [this formula](https://github.com/cuber/homebrew-libsecp256k1) formula from GitHub. To install it:

```
brew install --build-from-source path/to/blockchain-k-plugin/deps/libsecp256k1.rb
```

In case the packaged version of `libsecp256k1` is too old or none is available,
we include a pinned version as a submodule. See the details above.

##### Manual Installation

Install the other software

- bison
- flex
- cmake
- llvm
- clang
- crypto++
- gflags
- gmp
- jemalloc
- mpfr
- protobuf
- libsecp256k1
- openssl
- libyaml
- pkg-config
- rapidjson
- zlib

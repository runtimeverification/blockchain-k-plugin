Blockchain K Plugin
===================

For integrating cryptographic hooks into your K definition.

Building
--------

These instructions are for Ubuntu, assuming that Clang12 is installed.

- Update submodules: `git submodule update --init --recursive`
- Build dependencies: `make CC=clang-14 CXX=clang++-14 -j4 libff libcryptopp libsecp256k1 c-kzg-eip4844`
- Build: `make CC=clang-14 CXX=clang++-14 build -j4`

Testing
-------

- Run tests: `make test`

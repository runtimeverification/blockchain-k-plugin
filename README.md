Blockchain K Plugin
===================

For integrating cryptographic hooks into your K definition.

Building
--------

These instructions are for Ubuntu, assuming that Clang12 is installed.

- Update submodules: `git submodule update --init --recursive`
- Build dependencies: `make CXX=clang++-12 -j3 libff libcryptopp libsecp256k1`
- Build: `make CXX=clang++-12 build -j3`

Testing
-------

- Run tests: `make -C tests`

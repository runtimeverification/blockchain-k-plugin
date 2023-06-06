# set default install prefix if not set
PREFIX ?= $(CURDIR)/build

K_RELEASE ?= $(dir $(shell which kompile))..

LIBFF_CMAKE_FLAGS += -DCMAKE_CXX_FLAGS=-fPIC

# set OS specific defaults
ifeq ($(shell uname -s),Darwin)
# 1. OSX doesn't have /proc/ filesystem
# 2. fix cmake openssl detection for brew
SSL_ROOT ?= $(shell brew --prefix openssl)
LIBFF_CMAKE_FLAGS += -DWITH_PROCPS=OFF \
                     -DOPENSSL_ROOT_DIR=$(SSL_ROOT)
else
# llvm-backend code doesn't play nice with g++
export CXX := $(if $(findstring default, $(origin CXX)), clang++, $(CXX))
endif

INCLUDES := -I $(K_RELEASE)/include/kllvm -I $(PREFIX)/libcryptopp/include -I $(PREFIX)/libff/include -I $(PREFIX)/libsecp256k1/include -I dummy-version -I plugin -I plugin-c -I deps/cpp-httplib
CPPFLAGS += --std=c++14 $(INCLUDES)

ifneq ($(APPLE_SILICON),)
    LIBFF_CMAKE_FLAGS += -DCURVE=ALT_BN128 -DUSE_ASM=Off

    GMP_PREFIX ?= $(shell brew --prefix gmp)
    MPFR_PREFIX ?= $(shell brew --prefix mpfr)
    OPENSSL_PREFIX ?= $(shell brew --prefix openssl)
    CRYPTOPP_PREFIX ?= $(shell brew --prefix cryptopp@8.6.0)

    INCLUDES += -I $(GMP_PREFIX)/include -I $(MPFR_PREFIX)/include -I $(OPENSSL_PREFIX)/include -I $(CRYPTOPP_PREFIX)/include
endif

.PHONY: build libcryptopp libff libsecp256k1
build: plugin-c/json.o plugin-c/blake2.o plugin-c/crypto.o plugin-c/plugin_util.o plugin-c/k.o

.PHONY: install clean

DESTDIR         ?=
INSTALL_PREFIX  ?= /usr/local
INSTALL_DIR     := $(DESTDIR)$(INSTALL_PREFIX)
INSTALL_INCLUDE := $(INSTALL_DIR)/include/kframework

PLUGIN_NAMESPACE := blockchain-k-plugin
K_SOURCES := krypto.md

install: $(patsubst %, $(INSTALL_INCLUDE)/$(PLUGIN_NAMESPACE)/%, $(K_SOURCES))

$(INSTALL_INCLUDE)/$(PLUGIN_NAMESPACE)/%.md: plugin/%.md
	@mkdir -p $(dir $@)
	cp $< $@

clean:
	rm -rf */*.o */*/*.o build deps/libff/build
	cd deps/secp256k1 && test ! -f Makefile || $(MAKE) clean

# libcryptopp

libcryptopp: $(PREFIX)/libcryptopp/lib/libcryptopp.a
$(PREFIX)/libcryptopp/lib/libcryptopp.a:
	cd deps/cryptopp                      \
	  && $(MAKE)                          \
	  && $(MAKE) install PREFIX=$(PREFIX)/libcryptopp

# libff

libff: $(PREFIX)/libff/lib/libff.a
$(PREFIX)/libff/lib/libff.a:
	cd deps/libff                                                 \
	  && cmake . -DCMAKE_INSTALL_PREFIX=$(PREFIX)/libff $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE)                                                        \
	  && $(MAKE) install

# libsecp256k1

libsecp256k1: $(PREFIX)/libsecp256k1/lib/pkgconfig/libsecp256k1.pc
$(PREFIX)/libsecp256k1/lib/pkgconfig/libsecp256k1.pc:
	cd deps/secp256k1/                                             \
	    && ./autogen.sh                                            \
	    && CFLAGS='-fPIC' ./configure --enable-module-recovery --prefix=$(PREFIX)/libsecp256k1 \
	    && $(MAKE)                                                 \
	    && $(MAKE) install

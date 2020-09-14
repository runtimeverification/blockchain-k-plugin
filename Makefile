# set default install prefix if not set
PREFIX ?= $(CURDIR)/build

K_RELEASE ?= $(dir $(shell which kompile))..

# set OS specific defaults
ifeq ($(shell uname -s),Darwin)
# 1. OSX doesn't have /proc/ filesystem
# 2. fix cmake openssl detection for brew
LIBFF_CMAKE_FLAGS += -DWITH_PROCPS=OFF \
                     -DOPENSSL_ROOT_DIR=/usr/local/opt/$(shell brew desc openssl | cut -f1 -d:)
else
# llvm-backend code doesn't play nice with g++
export CXX := $(if $(findstring default, $(origin CXX)), clang++, $(CXX))
endif

INCLUDES := -I $(K_RELEASE)/include/kllvm -I $(PREFIX)/include -I dummy-version -I plugin -I plugin-c -I deps/cpp-httplib
CPPFLAGS += --std=c++14 $(INCLUDES)

.PHONY: build libcryptopp libff libsecp256k1
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/plugin_util.o plugin-c/world.o

plugin-c/blockchain.o: plugin-c/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

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
	rm -rf */*.o */*/*.o plugin/proto/*.pb.* build deps/libff/build
	cd deps/secp256k1 && $(MAKE) clean

# libcryptopp

libcryptopp: $(PREFIX)/lib/libcryptopp.a
$(PREFIX)/lib/libcryptopp.a:
	cd deps/cryptopp                      \
	  && $(MAKE)                          \
	  && $(MAKE) install PREFIX=$(PREFIX)

# libff

libff: $(PREFIX)/lib/libff.a
$(PREFIX)/lib/libff.a:
	@mkdir -p deps/libff/build
	cd deps/libff/build                                                 \
	  && cmake .. -DCMAKE_INSTALL_PREFIX=$(PREFIX) $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE)                                                        \
	  && $(MAKE) install

# libsecp256k1

libsecp256k1: $(PREFIX)/lib/pkgconfig/libsecp256k1.pc
$(PREFIX)/lib/pkgconfig/libsecp256k1.pc:
	cd deps/secp256k1/                                             \
	    && ./autogen.sh                                            \
	    && ./configure --enable-module-recovery --prefix=$(PREFIX) \
	    && $(MAKE)                                                 \
	    && $(MAKE) install

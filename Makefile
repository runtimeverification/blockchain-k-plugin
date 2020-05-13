
# set default install prefix if not set
PREFIX ?= "$(CURDIR)/build"

# set OSX specific flags
ifeq ($(shell uname -s),Darwin)
# look for K framework as a brew package
K_RELEASE ?= /usr/local/lib/kframework
# add brew includes path
INCLUDES += -I /usr/local/include
 # prefer brew clang because it is more recent than apple clang
ifeq "$(origin CC)"  "default"
export CC := /usr/local/opt/llvm/bin/clang
endif
ifeq "$(origin CXX)" "default"
export CXX := /usr/local/opt/llvm/bin/clang++
endif
# fix build errors on OSX for libff
# 1. libff cmake script fails to find brew installed openssl; force it to look at OPENSSL_ROOT_DIR
OPENSSL_VER := $(shell brew desc openssl | cut -f1 -d:)
LIBFF_EXPORTS := OPENSSL_ROOT_DIR=/usr/local/opt/$(OPENSSL_VER)
# 2. libff on osx must be compiled support for /proc filesystem disabled
LIBFF_CMAKE_FLAGS += -DWITH_PROCPS=OFF
# set Linux flags
else
K_RELEASE ?= /usr/lib/kframework
endif

CPPFLAGS += -I $(join $(K_RELEASE), /include) $(INCLUDES) -I $(join $(PREFIX), /include)

OBJS := client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o

.PHONY: build build-check clean cpp-httplib libcryptopp libff libsecp256k1

build-deps: cpp-httplib libcryptopp libff
build-test: CPPFLAGS += -I dummy-version -I plugin -I plugin-c
build-test: build-check build-deps $(OBJS)

build-check:
	@$(if $(wildcard $(K_RELEASE)/include),,$(error folder $$(K_RELEASE)/include does not exist and default K release folder not found; set K_RELEASE to point to your K installation path))

plugin-c/blockchain.o: plugin/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

clean:
	rm -rf */*.o */*/*.o plugin/proto/*.pb.* build
	git submodule deinit -f deps

# dependencies

# cpp-httplib

cpp-httplib: $(PREFIX)/include/httplib.h
$(PREFIX)/include/httplib.h: deps/cpp-httplib/httplib.h
	@mkdir -p "$(PREFIX)/include"
	cp deps/cpp-httplib/httplib.h "$(PREFIX)/include/"

# libcryptopp

libcryptopp: $(PREFIX)/lib/libcryptopp.a
$(PREFIX)/lib/libcryptopp.a: deps/cryptopp
	cd deps/cryptopp                      \
	  && $(MAKE)                          \
	  && $(MAKE) install PREFIX=$(PREFIX)

# libff

libff: $(PREFIX)/lib/libff.a
$(PREFIX)/lib/libff.a: deps/libff
	@mkdir -p deps/libff/build
	cd deps/libff/build                                                   \
	  && $(LIBFF_EXPORTS)                                                 \
	       cmake .. -DCMAKE_INSTALL_PREFIX=$(PREFIX) $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE) install

# libsecp256k1

libsecp256k1: $(PREFIX)/lib/pkgconfig/libsecp256k1.pc
$(PREFIX)/lib/pkgconfig/libsecp256k1.pc:
	cd deps/secp256k1/                                               \
	    && ./autogen.sh                                              \
	    && ./configure --enable-module-recovery --prefix="$(PREFIX)" \
	    && $(MAKE)                                                   \
	    && $(MAKE) install


# set OS specific flags
ifeq ($(shell uname -s),Darwin)
K_RELEASE ?= /usr/local/lib/kframework  # look for K framework as a brew package
INCLUDES += -I /usr/local/include       # add brew includes path
CXX ?= /usr/local/opt/llvm/bin/clang++  # prefer brew clang++ because it is more recent than apple clang
# fix build errors on OSX for libff
# 1. libff cmake script fails to find brew installed openssl; force it to look at OPENSSL_ROOT_DIR
OPENSSL_VER = $(shell brew desc openssl | cut -f1 -d:)
LIBFF_EXPORTS = OPENSSL_ROOT_DIR=/usr/local/opt/openssl@$(OPENSSL_VER)
# 2. libff on osx must be compiled support for /proc filesystem disabled
LIBFF_CONF_FLAGS += -DWITH_PROCPS=OFF
else
K_RELEASE ?= /usr/lib/kframework
CXX ?= clang++                          # use system clang++
endif

CPPFLAGS += -I $(join $(K_RELEASE), /include) $(INCLUDES) -I build/include

OBJS := client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o

.PHONY: build build-check clean cpp-httplib libcryptopp libff

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

cpp-httplib: build/include/httplib.h
build/include/httplib.h: deps/cpp-httplib/httplib.h
	@mkdir -p build/include
	cp deps/cpp-httplib/httplib.h build/include/

# libcryptopp

libcryptopp: build/lib/libcryptopp.a
build/lib/libcryptopp.a: deps/cryptopp
	cd deps/cryptopp && $(MAKE) && $(MAKE) install PREFIX=$(CURDIR)/build

# libff

libff: build/lib/libff.a
build/lib/libff.a: deps/libff
	@mkdir -p deps/libff/build
	cd deps/libff/build && $(LIBFF_EXPORTS) cmake .. -DCMAKE_INSTALL_PREFIX=$(CURDIR)/build $(LIBFF_CONF_FLAGS) && $(MAKE) install

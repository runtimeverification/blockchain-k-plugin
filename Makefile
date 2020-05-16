# set default install prefix if not set
PREFIX ?= $(CURDIR)/build
# set OS specific defaults
# 1. K installation
# 2. include path
# 3. C++ compiler
# 4. extra flags need for libff compilation
ifeq ($(shell uname -s),Darwin)
K_RELEASE ?= /usr/local/lib/kframework
CPPFLAGS += -I /usr/local/include
LIBFF_CMAKE_FLAGS += -DWITH_PROCPS=OFF -DOPENSSL_ROOT_DIR=/usr/local/opt/$(shell brew desc openssl | cut -f1 -d:)
else
K_RELEASE ?= /usr/lib/kframework
# llvm-backend code doesn't play nice with g++
export CXX := $(if $(findstring default, $(origin CXX)), clang++, $(CXX))
endif

INCLUDES := -I $(K_RELEASE)/include -I $(PREFIX)/include -I dummy-version -I plugin -I plugin-c -I deps/cpp-httplib
CPPFLAGS += --std=c++14 $(INCLUDES)

.PHONY: build libff
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o

plugin-c/blockchain.o: plugin/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o plugin/proto/*.pb.* build deps/libff/build

# libff

libff: $(PREFIX)/lib/libff.a
$(PREFIX)/lib/libff.a:
	@mkdir -p deps/libff/build
	cd deps/libff/build                                                 \
	  && cmake .. -DCMAKE_INSTALL_PREFIX=$(PREFIX) $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE)                                                        \
	  && $(MAKE) install

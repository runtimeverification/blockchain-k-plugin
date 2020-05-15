# set default install prefix if not set
PREFIX ?= $(CURDIR)/build
# set default K distribution location if not set
ifeq ($(shell uname -s),Darwin)
K_RELEASE ?= /usr/local/lib/kframework
else
K_RELEASE ?= /usr/lib/kframework
endif


CPPFLAGS += -I $(K_RELEASE)/include -I $(PREFIX)/include -I dummy-version -I plugin -I plugin-c -I deps/cpp-httplib
CXX = clang++-8

.PHONY: build libff
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o

plugin-c/blockchain.o: plugin/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o plugin/proto/*.pb.* build

# libff

libff: $(PREFIX)/lib/libff.a
$(PREFIX)/lib/libff.a:
	@mkdir -p deps/libff/build
	cd deps/libff/build                                                 \
	  && cmake .. -DCMAKE_INSTALL_PREFIX=$(PREFIX) $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE)                                                        \
	  && $(MAKE) install

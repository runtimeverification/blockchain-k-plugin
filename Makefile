# set default install prefix if not set
PREFIX ?= $(CURDIR)/build

K_INCLUDE ?= $(shell llvm-kompile --include-dir)

DESTDIR         ?=
INSTALL_PREFIX  ?= /usr/local
INSTALL_DIR     := $(DESTDIR)$(INSTALL_PREFIX)
INSTALL_INCLUDE := $(INSTALL_DIR)/include/kframework

PLUGIN_NAMESPACE := blockchain-k-plugin
K_SOURCES := krypto.md

.PHONY: build
build: krypto

.PHONY: install
install: $(patsubst %, $(INSTALL_INCLUDE)/$(PLUGIN_NAMESPACE)/%, $(K_SOURCES))

$(INSTALL_INCLUDE)/$(PLUGIN_NAMESPACE)/%.md: plugin/%.md
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o build deps/libff/build

.PHONY: test
test: build
	$(MAKE) -C krypto test-integration


# -----------
# libcryptopp
# -----------

.PHONY: libcryptopp
libcryptopp: $(PREFIX)/libcryptopp/lib/libcryptopp.a
$(PREFIX)/libcryptopp/lib/libcryptopp.a:
	cd deps/cryptopp                      \
	  && $(MAKE)                          \
	  && $(MAKE) install PREFIX=$(PREFIX)/libcryptopp


# -----
# libff
# -----

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

ifneq ($(APPLE_SILICON),)
    LIBFF_CMAKE_FLAGS += -DCURVE=ALT_BN128 -DUSE_ASM=Off
endif

.PHONY: libff
libff: $(PREFIX)/libff/lib/libff.a
$(PREFIX)/libff/lib/libff.a:
	cd deps/libff                                                 \
	  && cmake . -DCMAKE_INSTALL_PREFIX=$(PREFIX)/libff $(LIBFF_CMAKE_FLAGS) \
	  && $(MAKE)                                                        \
	  && $(MAKE) install


# ------
# blake2
# ------

.PHONY: blake2
blake2: $(PREFIX)/blake2/lib/blake2.a

CXXFLAGS=-O3
ifeq ($(shell uname -p),x86_64)
$(PREFIX)/blake2/lib/blake2.a: CXXFLAGS+=-mavx2
endif
$(PREFIX)/blake2/lib/blake2.a: plugin-c/blake2-compress.o plugin-c/blake2-avx2.o plugin-c/blake2-generic.o
	mkdir -p $(dir $@)
	ar qs $@ $^


# --------
# plugin-c
# --------

INCLUDES := -I $(K_INCLUDE)/kllvm -I $(K_INCLUDE) -I $(PREFIX)/libcryptopp/include -I $(PREFIX)/libff/include -I dummy-version -I plugin -I plugin-c -I deps/cpp-httplib

ifneq ($(APPLE_SILICON),)
    GMP_PREFIX ?= $(shell brew --prefix gmp)
    MPFR_PREFIX ?= $(shell brew --prefix mpfr)
    OPENSSL_PREFIX ?= $(shell brew --prefix openssl)
    CRYPTOPP_PREFIX ?= $(shell brew --prefix cryptopp@8.6.0)
    SECP256K1_PREFIX ?= $(shell brew --prefix secp256k1)
    BOOST_PREFIX ?= $(shell brew --prefix boost)
    INCLUDES += -I $(GMP_PREFIX)/include -I $(MPFR_PREFIX)/include -I $(OPENSSL_PREFIX)/include -I $(CRYPTOPP_PREFIX)/include -I $(SECP256K1_PREFIX)/include -I $(BOOST_PREFIX)/include
endif

CPPFLAGS += --std=c++17 -fPIC -O3 $(INCLUDES)

plugin-c/%.o: plugin-c/%.cpp $(PREFIX)/libcryptopp/lib/libcryptopp.a $(PREFIX)/libff/lib/libff.a
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

$(PREFIX)/plugin/lib/plugin.a: plugin-c/crypto.o plugin-c/hash_ext.o plugin-c/json.o plugin-c/k.o plugin-c/plugin_util.o
	mkdir -p $(dir $@)
	ar r $@ $^

.PHONY: plugin
plugin: $(PREFIX)/plugin/lib/plugin.a


# ------
# krypto
# ------

$(PREFIX)/krypto/lib/krypto.a: $(PREFIX)/libff/lib/libff.a $(PREFIX)/libcryptopp/lib/libcryptopp.a $(PREFIX)/blake2/lib/blake2.a $(PREFIX)/plugin/lib/plugin.a
	$(eval TMP := $(shell mktemp -d))
	for lib in $^; do                \
	    (cd $(TMP); ar x $$lib;) \
	done
	mkdir -p $(dir $@)
	ar r $@ $(TMP)/*.o
	rm -rf $(TMP)

.PHONY: krypto
krypto: $(PREFIX)/krypto/lib/krypto.a

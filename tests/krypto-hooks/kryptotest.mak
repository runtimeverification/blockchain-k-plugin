MAKEFILE_PATH1 := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

KOMPILE_FLAGS+=-I $(MAKEFILE_PATH1)../../plugin
KOMPILE_FLAGS+=--hook-namespaces "KRYPTO" --md-selector "k | libcrypto-extra"

PLUGIN_C=$(abspath $(MAKEFILE_PATH1)../../plugin-c)
INCLUDES=-I$(abspath $(MAKEFILE_PATH1)../../build/libff/include) -I$(abspath $(MAKEFILE_PATH1)../../build/libcryptopp/include)
LIBRARIES=$(abspath $(MAKEFILE_PATH1)../../build/libff/lib/libff.a) $(abspath $(MAKEFILE_PATH1)../../build/libcryptopp/lib/libcryptopp.a) -lssl -lcrypto -lprocps -lsecp256k1
CPP_SOURCES=crypto.cpp plugin_util.cpp blake2.cpp hash_ext.cpp

KOMPILE_FLAGS+=$(addprefix -ccopt , $(INCLUDES) $(patsubst %, $(PLUGIN_C)/%, $(CPP_SOURCES)) $(LIBRARIES))

include $(dir $(shell which kompile))/../include/kframework/ktest.mak

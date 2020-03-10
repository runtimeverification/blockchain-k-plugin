CPPFLAGS += -I llvm-backend/build/include -I llvm-backend/include -I vm-c -I dummy-version -I plugin -I vm-c/kevm -I plugin-c -I libff/build/install/include -I deps/uwebsockets/src -I deps/uwebsockets/uSockets/src
CXX=clang++-8

.PHONY: build
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o vm-c/init.o vm-c/main.o vm-c/vm.o vm-c/kevm/semantics.o

plugin-c/blockchain.o: plugin/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o plugin/proto/*.pb.*

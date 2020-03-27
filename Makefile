CPPFLAGS += -I llvm-backend/build/include -I llvm-backend/include -I vm-c -I dummy-version -I plugin -I vm-c/kevm -I plugin-c -I libff/build/install/include -I deps/cpp-httplib
CXX=clang++-8

.PHONY: build
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/bn128.o plugin-c/secp256k1.o plugin-c/hash.o plugin-c/world.o

plugin-c/blockchain.o: plugin-c/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o plugin-c/proto/*.pb.*

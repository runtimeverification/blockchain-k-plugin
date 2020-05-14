CPPFLAGS += -I /usr/lib/kframework/include -I dummy-version -I plugin -I plugin-c -I libff/build/install/include -I deps/cpp-httplib
CXX=clang++-8

.PHONY: build
build: client-c/json.o client-c/main.o plugin-c/blake2.o plugin-c/blockchain.o plugin-c/crypto.o plugin-c/world.o

plugin-c/blockchain.o: plugin/proto/msg.pb.h

%.pb.h: %.proto
	protoc --cpp_out=. $<

.PHONY: clean
clean:
	rm -rf */*.o */*/*.o plugin/proto/*.pb.*

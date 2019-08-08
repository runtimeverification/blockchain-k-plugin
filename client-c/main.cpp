#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "runtime/alloc.h"
#include "version.h"
#include "init.h"

int main(int argc, char **argv) {
  std::string usage = std::string("Usage: ") + argv[0] + " PORT BIND_ADDRESS";
  if (argc == 2 && argv[1] == std::string("--help")) {
    std::cout << usage << std::endl;
    return 0;
  } else if (argc == 2 && argv[1] == std::string("--version")) {
    std::cout << argv[0] << " version " << VM_VERSION << std::endl;
    return 0;
  } else if (argc != 3) {
    std::cerr << "Incorrect number of arguments" << std::endl;
    std::cerr << usage << std::endl;
    return 1;
  }
  int port = std::stoi(argv[1]);
  in_addr host;
  if (!inet_aton(argv[2], &host)) {
    std::cerr << "Invalid bind address" << std::endl;
    return 1;
  }

  int sock = init(port, host);
  static uint64_t mode = (((uint64_t)getTagForSymbolName("LblNORMAL{}")) << 32) | 1;
  inj *modeinj = (inj *)koreAlloc(sizeof(inj));
  static blockheader hdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortMode{}, SortKItem{}}"));
  modeinj->h = hdr;
  modeinj->data = (block*)mode;
  uint64_t schedule = (((uint64_t)getTagForSymbolName("LblPETERSBURG_EVM{}")) << 32) | 1;
  inj *scheduleinj = (inj *)koreAlloc(sizeof(inj));
  static blockheader hdr2 = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortSchedule{}, SortKItem{}}"));
  scheduleinj->h = hdr2;
  scheduleinj->data = (block*)schedule;
  zinj *sockinj = (zinj *)koreAlloc(sizeof(zinj));
  static blockheader hdr3 = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortInt{}, SortKItem{}}"));
  sockinj->h = hdr3;
  mpz_t sock_z;
  mpz_init_set_si(sock_z, sock);
  sockinj->data = move_int(sock_z);
  static uint64_t accept = (((uint64_t)getTagForSymbolName("Lblaccept{}")) << 32) | 1;
  block *kcell = (block *)accept;
  map withSched = hook_MAP_element(configvar("$SCHEDULE"), (block *)scheduleinj);
  map withMode = hook_MAP_update(&withSched, configvar("$MODE"), (block *)modeinj);
  map withSocket = hook_MAP_update(&withMode, configvar("$SOCK"), (block *)sockinj);
  map init = hook_MAP_update(&withSocket, configvar("$PGM"), kcell);
  static uint32_t tag2 = getTagForSymbolName("LblinitGeneratedTopCell{}");
  void *arr[1];
  arr[0] = &init;
  block* init_config = (block *)evaluateFunctionSymbol(tag2, arr);
  block* final_config = take_steps(-1, init_config);
  printConfiguration("/dev/stderr", final_config);
}


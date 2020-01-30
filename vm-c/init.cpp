#include <iostream>
#include<cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "runtime/header.h"
#include "runtime/alloc.h"

#include "init.h"

int init(int port, in_addr host) {
  initStaticObjects();

  unlink(SOCKET_NAME);

  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(1);
  }

  sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  int sec = 0;
  int ret;
  do {
    if (sec > 0) {
      std::cerr << "Socket in use, retrying in " << sec << "..." << std::endl;
    }
    sleep(sec);
    ret = bind(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    sec++;
  } while(ret == -1 && errno == EADDRINUSE);
  if (ret) {
    perror("bind");
    exit(1);
  }
  if (listen(sock, 50)) {
    perror("listen");
    exit(1);
  }
  set_gc_interval(10000);
  return sock;
}

block* configvar(const char *name) {
  stringinj* inj = (stringinj*)koreAlloc(sizeof(stringinj));
  inj->data = makeString(name);
  static uint32_t tag = getTagForSymbolName("inj{SortKConfigVar{}, SortKItem{}}");
  inj->h = getBlockHeaderForSymbol(tag);
  return (block*)inj;
}



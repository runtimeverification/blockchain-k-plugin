#include <iostream>
#include<cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "runtime/header.h"
#include "runtime/alloc.h"

#include "k.h"

block* configvar(const char *name) {
  stringinj* inj = (stringinj*)koreAlloc(sizeof(stringinj));
  inj->data = makeString(name);
  static uint32_t tag = getTagForSymbolName("inj{SortKConfigVar{}, SortKItem{}}");
  inj->h = getBlockHeaderForSymbol(tag);
  return (block*)inj;
}

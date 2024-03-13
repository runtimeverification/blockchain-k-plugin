#include "k.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include "runtime/alloc.h"
#include "runtime/header.h"

block* configvar(const char* name) {
  stringinj* inj = (stringinj*)kore_alloc(sizeof(stringinj));
  inj->data = makeString(name);
  static uint32_t tag =
      get_tag_for_symbol_name("inj{SortKConfigVar{}, SortKItem{}}");
  inj->h = get_block_header_for_symbol(tag);
  return (block*)inj;
}

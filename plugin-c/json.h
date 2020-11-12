#ifndef JSON_H
#define JSON_H

#include <netinet/in.h>
#include "runtime/header.h"
#include <gmp.h>

extern "C" {
  string *hook_STRING_int2string(mpz_ptr);
}

struct zinj {
  blockheader h;
  mpz_ptr data;
};

struct inj {
  blockheader h;
  block* data;
};

struct stringinj {
  blockheader h;
  string *data;
};

typedef struct boolinj {
  struct blockheader h;
  bool data;
} boolinj;

struct jsonlist {
  blockheader h;
  block *hd;
  jsonlist* tl;
};

struct json {
  blockheader h;
  jsonlist *data;
};

struct jsonmember {
  blockheader h;
  block *key;
  block *val;
};

#endif

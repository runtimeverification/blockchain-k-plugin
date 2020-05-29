#ifndef PLUGIN_UTIL_H
#define PLUGIN_UTIL_H

#include <cstdint>
#include <cstring>
#include <gmp.h>
#include "runtime/alloc.h"
#include "runtime/header.h"

extern "C" {
inline string* allocString(size_t len);
string *hexEncode(unsigned char *digest, size_t len);
string *raw(unsigned char *digest, size_t len);
}

#endif

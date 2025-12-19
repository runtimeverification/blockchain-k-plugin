#ifndef P256VERIFY_H
#define P256VERIFY_H

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>

#include "plugin_util.h"

extern "C" {
    struct string *hook_KRYPTO_p256verify(struct string *input);
}

#endif
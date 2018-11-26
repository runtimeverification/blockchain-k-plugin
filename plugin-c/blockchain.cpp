#include <gmp.h>
#include <gmpxx.h>
#include <map>
#include "proto/msg.pb.h"
#include "world.h"
#include "runtime/header.h"

using namespace io::iohk::ethereum::extvm;

static std::map<mpz_class, Account *> accounts;
static std::map<mpz_class, std::map<mpz_class, StorageData *>> storage;
static std::map<mpz_class, Code *> code;
static std::map<mpz_class, Blockhash *> blockhash;

void clear_cache() {
  accounts.clear();
  storage.clear();
  code.clear();
  blockhash.clear();
}

static Account* get_account(mpz_t acctID) {
  Account* acct = accounts[mpz_class(acctID)];
  if (!acct) {
    acct = World::get_account(of_z_width(20, acctID));
  }
  return acct;
}

static StorageData* get_storage_data(mpz_t acctID, mpz_t index) {
  StorageData *data = storage[mpz_class(acctID)][mpz_class(index)];
  if (!data) {
    data = World::get_storage_data(of_z_width(20, acctID), of_z(index));
  }
  return data;
}

static Code* get_code(mpz_t acctID) {
  Code* c = code[mpz_class(acctID)];
  if (!c) {
    c = World::get_code(of_z_width(20, acctID));
  }
  return c;
}

static Blockhash* get_blockhash(mpz_t offset) {
  Blockhash* h = blockhash[mpz_class(offset)];
  if (!h) {
    h = World::get_blockhash(mpz_get_ui(offset));
  }
  return h;
}

extern "C" {

string* makeString(const char*, ssize_t);

mpz_ptr hook_BLOCKCHAIN_getBalance(mpz_t acctID) {
  Account *acct = get_account(acctID);
  return to_z_unsigned(acct->balance());
}

mpz_ptr hook_BLOCKCHAIN_getNonce(mpz_t acctID) {
  Account *acct = get_account(acctID);
  return to_z_unsigned(acct->nonce());
}

bool hook_BLOCKCHAIN_isCodeEmpty(mpz_t acctID) {
  Account *acct = get_account(acctID);
  return acct->codeempty();
}

mpz_ptr hook_BLOCKCHAIN_getStorageData(mpz_t acctID, mpz_t index) {
  StorageData *data = get_storage_data(acctID, index);
  return to_z(data->data());
}

string* hook_BLOCKCHAIN_getCode(mpz_t acctID) {
  Code *code = get_code(acctID);
  return makeString(code->code().c_str(), code->code().size());
}

mpz_ptr hook_BLOCKCHAIN_getBlockhash(mpz_t offset) {
  Blockhash *h = get_blockhash(offset);
  return to_z_unsigned(h->hash());
}

}

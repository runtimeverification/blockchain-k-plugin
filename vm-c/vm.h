#ifndef VM_H
#define VM_H

extern "C" {
  size_t hook_LIST_size_long(list *l);
  block* hook_LIST_get_long(list *l, size_t i);
  list hook_MAP_values(map *m);
  list hook_MAP_keys_list(map *m);
  block* hook_MAP_lookup(map *m, block* key);
  map hook_MAP_element(block*, block*);
  map hook_MAP_update(map *, block*, block*);
  block* take_steps(uint32_t, block*);
  string* makeString(const char *, ssize_t len=-1);
  mpz_ptr hook_BLOCKCHAIN_getBalance(mpz_ptr);
  mpz_ptr hook_BLOCKCHAIN_getNonce(mpz_ptr);
  mpz_ptr hook_BLOCKCHAIN_getStorageData(mpz_ptr, mpz_ptr);
  bool hook_BLOCKCHAIN_accountExists(mpz_ptr);
  bool hook_BLOCKCHAIN_isCodeEmpty(mpz_ptr);
}

void clear_cache(void);

struct kseq {
  blockheader h;
  block* hd;
  uint64_t tl;
};

struct zinj {
  blockheader h;
  mpz_ptr data;
};

struct mapinj {
  blockheader h;
  map data;
};

struct inj {
  blockheader h;
  block* data;
};

struct account;

struct acctinj {
  blockheader h;
  account* data;
};

struct log;

struct loginj {
  blockheader h;
  log* data;
};

struct stringinj {
  blockheader h;
  string *data;
};

struct accounts {
  blockheader h;
  map data;
};

struct kcell {
  blockheader h;
  bool iscreate;
  mpz_ptr to;
  mpz_ptr from;
  string* code;
  block* args;
  mpz_ptr value;
  mpz_ptr gasprice;
  mpz_ptr gas;
  mpz_ptr beneficiary;
  mpz_ptr difficulty;
  mpz_ptr number;
  mpz_ptr gaslimit;
  mpz_ptr timestamp;
  string* function;
};

struct kcellinj {
  blockheader h;
  kcell* data;
};

#endif

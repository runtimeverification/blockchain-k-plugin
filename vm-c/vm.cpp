#include <gmp.h>
#include <iostream>
#include "proto/msg.pb.h"
#include "runtime/header.h"
#include "runtime/alloc.h"
#include "semantics.h"
#include "world.h"
#include "vm.h"

using namespace io::iohk::ethereum::extvm;

std::vector<mpz_ptr> k_to_z(list* l) {
  std::vector<mpz_ptr> result;
  for (size_t i = 0; i < hook_LIST_size_long(l); i++) {
     zinj* elem = (zinj*) hook_LIST_get_long(l, i);
     result.push_back(elem->data);
  }
  return result;
}

std::vector<account *> k_to_acct(map* m) {
  list l = hook_MAP_values(m);
  std::vector<account *> result;
  for (size_t i = 0; i < hook_LIST_size_long(&l); i++) {
     account* elem = (account*) hook_LIST_get_long(&l, i);
     result.push_back(elem);
  }
  return result;
}

std::vector<log *> k_to_log(list* l) {
  std::vector<log*> result;
  for (size_t i = 0; i < hook_LIST_size_long(l); i++) {
     loginj* elem = (loginj*) hook_LIST_get_long(l, i);
     result.push_back(elem->data);
  }
  return result;
}

void k_to_log(log* log, LogEntry *pb) {
  std::string address = of_z_width(20, log->acct);
  std::vector<mpz_ptr> topics = k_to_z(&log->topics);
  void *arr[1];
  arr[0] = logData(log);
  string* token = (string*)evaluateFunctionSymbol(unparseByteStack, arr);
  pb->set_address(address);
  for (mpz_ptr topic : topics) {
    pb->add_topics(of_z_width(32, topic));
  }
  pb->set_data(std::string(token->data, len(token)));
}

block* configvar(const char *name) {
  stringinj* inj = (stringinj*)koreAlloc(sizeof(stringinj));
  inj->data = makeString(name);
  uint32_t tag = getTagForSymbolName("inj{SortKConfigVar{}, SortKItem{}}");
  inj->h = getBlockHeaderForSymbol(tag);
  return (block*)inj;
}

const char* kcellinjName(void);

block* make_k_cell(bool iscreate, mpz_ptr to, mpz_ptr from, string *code, block *args, mpz_ptr value, mpz_ptr gasprice, mpz_ptr gas, mpz_ptr beneficiary, mpz_ptr difficulty, mpz_ptr number, mpz_ptr gaslimit, mpz_ptr timestamp, string *function) {
  kcellinj *inj = (kcellinj *)koreAlloc(sizeof(kcellinj));
  uint32_t tag = getTagForSymbolName(kcellinjName());
  inj->h = getBlockHeaderForSymbol(tag);
  kcell* runvm = (kcell *)koreAlloc(sizeof(kcell));
  inj->data = runvm;
  tag = getTagForSymbolName("LblrunVM{}");
  runvm->h = getBlockHeaderForSymbol(tag);
  runvm->iscreate = iscreate;
  runvm->to = to;
  runvm->from = from;
  runvm->code = code;
  runvm->args = args;
  runvm->value = value;
  runvm->gasprice = gasprice;
  runvm->gas = gas;
  runvm->beneficiary = beneficiary;
  runvm->difficulty = difficulty;
  runvm->number = number;
  runvm->gaslimit = gaslimit;
  runvm->timestamp = timestamp;
  runvm->function = function;
  return (block*)inj;
}

bool storage_is_modified(mpz_ptr acctID, map* storage) {
  list keys = hook_MAP_keys_list(storage);
  for (size_t i = 0; i < hook_LIST_size_long(&keys); i++) {
    zinj* key = (zinj*)hook_LIST_get_long(&keys, i);
    zinj* value = (zinj*)hook_MAP_lookup(storage, (block*)key);
    if (mpz_cmp(hook_BLOCKCHAIN_getStorageData(acctID, key->data), value->data) != 0) {
      return true;
    }
  }
  return false;
}

bool code_is_modified(mpz_ptr acctID, block* code) {
  uint32_t tag = getTagForSymbolName("LblaccountEmpty{}");
  void* arr[3];
  mpz_t zero;
  mpz_init(zero);
  arr[0] = code;
  arr[1] = zero;
  arr[2] = zero;
  bool isCodeEmpty = *(bool *)evaluateFunctionSymbol(tag, arr);
  return hook_BLOCKCHAIN_isCodeEmpty(acctID) && !isCodeEmpty;
}

bool account_is_modified(std::vector<mpz_ptr> selfdestruct, account *acct) {
  for (mpz_ptr deleted : selfdestruct) {
    if (mpz_cmp(deleted, acct->acctID->data) == 0) {
      return false;
    }
  }
  if (mpz_cmp(hook_BLOCKCHAIN_getBalance(acct->acctID->data), acct->balance->data) != 0) {
    return true;
  }
  if (mpz_cmp(hook_BLOCKCHAIN_getNonce(acct->acctID->data), acct->nonce->data) != 0) {
    return true;
  }
  if (code_is_modified(acct->acctID->data, acct->code->data)) {
    return true;
  }
  if (storage_is_modified(acct->acctID->data, &acct->storage->data)) {
    return true;
  }
  return false;
}

std::string get_code_bytes(block* code) {
  uint32_t tag = getTagForSymbolName("LblcontractBytes{}");
  void* arr[1];
  arr[0] = code;
  string *token = (string*)evaluateFunctionSymbol(tag, arr);
  return std::string(token->data, len(token));
}

void k_to_mod_acct(account* acct, ModifiedAccount* mod_acct) {
  std::string address = of_z_width(20, acct->acctID->data);
  std::string nonce = of_z(acct->nonce->data);
  std::string balance = of_z(acct->balance->data);
  std::string code;
  if (code_is_modified(acct->acctID->data, acct->code->data)) {
    code = get_code_bytes(acct->code->data);
  } else {
    code = "";
  }
  map* storage = &acct->storage->data;
  list keys = hook_MAP_keys_list(storage);
  for (size_t i = 0; i < hook_LIST_size_long(&keys); i++) {
    zinj* key = (zinj*)hook_LIST_get_long(&keys, i);
    zinj* value = (zinj*)hook_MAP_lookup(storage, (block*)key);
    if (mpz_cmp(hook_BLOCKCHAIN_getStorageData(acct->acctID->data, key->data), value->data) != 0) {
      StorageUpdate *update = mod_acct->add_storageupdates();
      update->set_offset(of_z(key->data));
      update->set_data(of_z(value->data));
    }
  }
  mod_acct->set_address(address);
  mod_acct->set_nonce(nonce);
  mod_acct->set_balance(balance);
  mod_acct->set_code(code);
}

input_data unpack_input(bool, std::string);
uint64_t get_schedule(mpz_ptr, CallContext*);
bool get_error(mpz_ptr);

CallResult run_transaction(CallContext ctx) {
  std::cerr << ctx.DebugString() << std::endl;
  clear_cache();
  bool iscreate = ctx.recipientaddr().size() == 0;
  mpz_ptr to = to_z_unsigned(ctx.recipientaddr());
  mpz_ptr from = to_z_unsigned(ctx.calleraddr());
  input_data in = unpack_input(iscreate, ctx.inputdata());
  mpz_ptr value = to_z_unsigned(ctx.callvalue());
  mpz_ptr gasprice = to_z_unsigned(ctx.gasprice());
  mpz_ptr gas = to_z_unsigned(ctx.gasprovided());
  mpz_ptr beneficiary = to_z_unsigned(ctx.blockheader().beneficiary());
  mpz_ptr difficulty = to_z_unsigned(ctx.blockheader().difficulty());
  mpz_ptr number = to_z_unsigned(ctx.blockheader().number());
  mpz_ptr gaslimit = to_z_unsigned(ctx.blockheader().gaslimit());
  mpz_t timestamp;
  mpz_init_set_ui(timestamp, ctx.blockheader().unixtimestamp());
  uint64_t mode = (((uint64_t)getTagForSymbolName("LblNORMAL{}")) << 32) | 1;
  inj *modeinj = (inj *)koreAlloc(sizeof(inj));
  modeinj->h = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortMode{}, SortKItem{}}"));
  modeinj->data = (block*)mode;
  uint64_t schedule = get_schedule(number, &ctx);
  inj *scheduleinj = (inj *)koreAlloc(sizeof(inj));
  scheduleinj->h = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortSchedule{}, SortKItem{}}"));
  scheduleinj->data = (block*)schedule;
  block* inj = make_k_cell(iscreate, to, from, in.code, in.args, value, gasprice, gas, beneficiary, difficulty, number, gaslimit, timestamp, in.function);
  uint32_t tag = getTagForSymbolName("kseq{}");
  map withSched = hook_MAP_element(configvar("$SCHEDULE"), (block *)scheduleinj);
  map withMode = hook_MAP_update(&withSched, configvar("$MODE"), (block *)modeinj);
  map init = hook_MAP_update(&withMode, configvar("$PGM"), inj);
  tag = getTagForSymbolName("LblinitGeneratedTopCell{}");
  void *arr[1];
  arr[0] = &init;
  block* init_config = (block *)evaluateFunctionSymbol(tag, arr);
  block* final_config = take_steps_no_depth(init_config);
  tag = getTagForSymbolName("LblextractConfig{}");
  arr[0] = final_config;
  tx_result* extracted = (tx_result *)evaluateFunctionSymbol(tag, arr);
  std::string ret_data = get_output_data(&extracted->rets);
  std::string gasLeft = of_z(extracted->gas);
  std::string refund = of_z(extracted->refund);
  std::string status = of_z(extracted->status);
  bool error = get_error(extracted->status);
  auto selfdestruct = k_to_z(&extracted->selfdestruct);
  auto touched = k_to_z(&extracted->touched);
  auto accounts = k_to_acct(&extracted->accounts->data);
  auto logs = k_to_log(&extracted->logs);
  CallResult result;
  result.set_returndata(ret_data);
  result.set_returncode(status);
  result.set_gasremaining(gasLeft);
  result.set_gasrefund(refund);
  result.set_error(error);
  for (mpz_ptr acct : selfdestruct) {
    result.add_deletedaccounts(of_z_width(20, acct));
  }
  for (mpz_ptr acct : touched) {
    result.add_touchedaccounts(of_z_width(20, acct));
  }
  for (account *acct : accounts) {
    if (account_is_modified(selfdestruct, acct)) {
      auto mod = result.add_modifiedaccounts();
      k_to_mod_acct(acct, mod);
    }
  }
  for (log *log : logs) {
    auto log_pb = result.add_logs();
    k_to_log(log, log_pb);
  }
  std::cerr << result.DebugString() << std::endl;
  return result;
}
 

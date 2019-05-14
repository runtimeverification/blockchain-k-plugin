#include "proto/msg.pb.h"
#include "runtime/header.h"
#include "semantics.h"
#include "world.h"
#include "vm.h"
#include <string>

using namespace org::kframework::kevm::extvm;

bool get_error(mpz_ptr status) {
  return mpz_sgn(status) == 0;
}

std::string get_output_data(string **sptr) {
  string* s = *sptr;
  return std::string(s->data, len(s));
}

uint64_t get_schedule(mpz_ptr number, CallContext *ctx) {
  mpz_ptr eip161 = to_z(ctx->ethereumconfig().eip161blocknumber());
  static uint32_t eip158_tag = getTagForSymbolName("LblEIP158'Unds'EVM{}");
  static uint32_t eip150_tag = getTagForSymbolName("LblEIP150'Unds'EVM{}");
  static uint32_t homestead_tag = getTagForSymbolName("LblHOMESTEAD'Unds'EVM{}");
  static uint32_t frontier_tag = getTagForSymbolName("LblFRONTIER'Unds'EVM{}");
  uint32_t tag;
  if (mpz_cmp(number, eip161) >= 0) {
    tag = eip158_tag;
  } else {
    mpz_ptr eip150 = to_z(ctx->ethereumconfig().eip150blocknumber());
    if (mpz_cmp(number, eip150) >= 0) {
      tag = eip150_tag;
    } else {
      mpz_ptr homestead = to_z(ctx->ethereumconfig().homesteadblocknumber());
      if (mpz_cmp(number, homestead) >= 0) {
        tag = homestead_tag;
      } else {
        tag = frontier_tag;
      }
    }
  }
  return (((uint64_t)tag) << 32) | 1;
}

input_data unpack_input(bool iscreate, std::string data) {
  input_data res;
  res.args = (block *)makeString(data.c_str(), data.size());
  res.code = makeString("");
  res.function = makeString("");
  return res;
}

uint32_t kcellInjTag = getTagForSymbolName("inj{SortEthereumSimulation{}, SortKItem{}}");
uint32_t unparseByteStack = getTagForSymbolName("LblunparseByteStack{}");

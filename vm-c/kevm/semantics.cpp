#include "proto/msg.pb.h"
#include "runtime/header.h"
#include "semantics.h"
#include "world.h"
#include "vm.h"
#include <string>

using namespace io::iohk::ethereum::extvm;

bool get_error(mpz_ptr status) {
  return mpz_sgn(status) == 0;
}

std::string get_output_data(string **sptr) {
  string* s = *sptr;
  return std::string(s->data, len(s));
}

uint64_t get_schedule(mpz_ptr number, CallContext *ctx) {
  mpz_ptr eip161 = to_z(ctx->ethereumconfig().eip161blocknumber());
  const char *name;
  if (mpz_cmp(number, eip161) >= 0) {
    name = "LblEIP158'Unds'EVM{}";
  } else {
    mpz_ptr eip150 = to_z(ctx->ethereumconfig().eip150blocknumber());
    if (mpz_cmp(number, eip150) >= 0) {
      name = "LblEIP150'Unds'EVM{}";
    } else {
      mpz_ptr homestead = to_z(ctx->ethereumconfig().homesteadblocknumber());
      if (mpz_cmp(number, homestead) >= 0) {
        name = "LblHOMESTEAD'Unds'EVM{}";
      } else {
        name = "LblFRONTIER'Unds'EVM{}";
      }
    }
  }
  return (((uint64_t)getTagForSymbolName(name)) << 32) | 1;
}

input_data unpack_input(bool iscreate, std::string data) {
  input_data res;
  res.args = (block *)makeString(data.c_str(), data.size());
  res.code = makeString("");
  res.function = makeString("");
  return res;
}

const char *kcellinjName() {
  return "inj{SortEthereumSimulation{}, SortKItem{}}";
}

uint32_t unparseByteStack = getTagForSymbolName("LblunparseByteStack{}");

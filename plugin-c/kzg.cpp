#include "plugin_util.h"
#include "eip4844/eip4844.h"
#include "setup/setup.h"

extern "C" {

static void setup(KZGSettings *s) {
    FILE *fp;
    C_KZG_RET ret;

    /* Open the mainnet trusted setup file */
    fp = fopen("deps/k-czg-4844/trusted_setup.txt", "r");
    if(fp == NULL){
      throw std::runtime_error("unable to open setup file");
    }

    /* Load the trusted setup file */
    ret = load_trusted_setup_file(s, fp, 0);
    if(ret != C_KZG_OK) {
      throw std::runtime_error("unable to load trusted setup file");
    }

    fclose(fp);
}

bool hook_KRYPTO_verifyKZGProof(struct string *commitment, struct string *z, struct string *y, struct string *proof) {
  KZGSettings settings;
  static thread_local bool once = true;
  if (once) {
      setup(&settings);
      once = false;
  }
  if (len(commitment) != 48) {
      std::invalid_argument("Length of commitment is not 42 bytes");
  }
  if (len(z) != 32) {
      std::invalid_argument("Length of z field is not 32 bytes");
  }
  if (len(y) != 32) {
      std::invalid_argument("Length of y field is not 32 bytes");
  }
  if (len(proof) != 48) {
      std::invalid_argument("Length of proof is not 42 bytes");
  }
  bool res;
  verify_kzg_proof(&res, (Bytes48 *)&commitment->data[0], (Bytes32 *)&z->data[0], (Bytes32 *)&y->data[0], (Bytes48 *)&proof->data[0], &settings);
  return res;
}

}

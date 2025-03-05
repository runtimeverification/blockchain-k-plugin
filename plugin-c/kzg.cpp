#include "eip4844/eip4844.h"
#include "plugin_util.h"
#include "setup/setup.h"

extern const char *trusted_setup_str;

extern "C" {

static void setup(KZGSettings *s) {
  FILE *fp;
  C_KZG_RET ret;

  /* Create temporary file to write the contents of mainnet trusted setup string
   */
  fp = tmpfile();
  fputs(trusted_setup_str, fp);

  /* Rewind the mainnet trusted setup file */
  rewind(fp);

  /* Load the trusted setup file */
  ret = load_trusted_setup_file(s, fp, 0);
  if (ret != C_KZG_OK) {
    throw std::runtime_error("unable to load trusted setup file");
  }

  fclose(fp);
}

bool hook_KRYPTO_verifyKZGProof(struct string *commitment, struct string *z,
                                struct string *y, struct string *proof) {
  static thread_local KZGSettings settings;
  static thread_local bool once = true;
  if (once) {
    setup(&settings);
    once = false;
  }
  bool res;
  verify_kzg_proof(&res, (Bytes48 *)&commitment->data[0],
                   (Bytes32 *)&z->data[0], (Bytes32 *)&y->data[0],
                   (Bytes48 *)&proof->data[0], &settings);
  return res;
}
}

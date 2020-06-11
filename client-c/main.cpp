#include <iostream>
#include <regex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#define CPPHTTPLIB_THREAD_POOL_COUNT 1
#include <httplib.h>
#include "runtime/alloc.h"
#include "version.h"
#include "init.h"
#include <gflags/gflags.h>

void runKServer(httplib::Server *svr);
void countBrackets(const char *buffer, size_t len);
bool doneReading (const char *buffer, int len);
void writeWithPrefix (int, std::string, std::string);

static bool K_SHUTDOWNABLE;
static bool K_NOTIFICATIONS;
static uint32_t K_SCHEDULE_TAG;
static int K_CHAINID;
static int K_DEPTH;
static int K_WRITE_FD;
static int K_READ_FD;

int brace_counter_, bracket_counter_, object_counter_;

static std::string FRONTIER = "frontier";
static std::string HOMESTEAD = "homestead";
static std::string TANGERINE_WHISTLE = "tangerine_whistle";
static std::string SPURIOUS_DRAGON = "spurious_dragon";
static std::string BYZANTIUM = "byzantium";
static std::string CONSTANTINOPLE = "constantinople";
static std::string PETERSBURG = "petersburg";
static std::string ISTANBUL = "istanbul";

DEFINE_int32(port, 8545, "Port to listen on with HTTP protocol");
DEFINE_int32(depth, -1, "For debugging, stop execution at a certain depth.");
DEFINE_string(host, "localhost", "IP/Hostname to bind to");
DEFINE_bool(shutdownable, false, "Allow `firefly_shutdown` message to kill server");
DEFINE_bool(dump, false, "Dump the K Server configuration on shutdown");
DEFINE_bool(dump_rpc, false, "Dump RPC messages to file specified by --dump-rpc-file");
DEFINE_string(dump_rpc_file, "/dev/stdout", "File to dump RPC messages to");
DEFINE_bool(respond_to_notifications, false, "Respond to incoming notification messages as normal messages");
DEFINE_string(hardfork, "istanbul", "Ethereum client hardfork. Supported: 'frontier', "
             "'homestead', 'tangerine_whistle', 'spurious_dragon', 'byzantium', "
             "'constantinople', 'petersburg', 'istanbul'");
DEFINE_int32(networkId, 28346, "Set network chain id");
DEFINE_bool(vmversion, false, "Display current VM version");

int main(int argc, char **argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  K_SCHEDULE_TAG = getTagForSymbolName("LblISTANBUL'Unds'EVM{}");

  if (FLAGS_vmversion) {
    std::cout << argv[0] << " version " << VM_VERSION << std::endl;
    return 0;
  }

  K_CHAINID = FLAGS_networkId;
  K_SHUTDOWNABLE = FLAGS_shutdownable;
  K_DEPTH = FLAGS_depth;
  K_NOTIFICATIONS = FLAGS_respond_to_notifications;

  if (FLAGS_hardfork == FRONTIER) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblFRONTIER'Unds'EVM{}");
  } else if (FLAGS_hardfork == HOMESTEAD) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblHOMESTEAD'Unds'EVM{}");
  } else if (FLAGS_hardfork == TANGERINE_WHISTLE) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblTANGERINE'Unds'WHISTLE'Unds'EVM{}");
  } else if (FLAGS_hardfork == SPURIOUS_DRAGON) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblSPURIOUS'Unds'DRAGON'Unds'EVM{}");
  } else if (FLAGS_hardfork == BYZANTIUM) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblBYZANTIUM'Unds'EVM{}");
  } else if (FLAGS_hardfork == CONSTANTINOPLE) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblCONSTANTINOPLE'Unds'EVM{}");
  } else if (FLAGS_hardfork == PETERSBURG) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblPETERSBURG'Unds'EVM{}");
  } else if (FLAGS_hardfork == ISTANBUL) {
    K_SCHEDULE_TAG = getTagForSymbolName("LblISTANBUL'Unds'EVM{}");
  } else {
      std::cerr << "Invalid hardfork found: " << FLAGS_hardfork << std::endl;
      return 1;
  }

  httplib::Server svr;

  // Start KServer in a separate thread
  std::thread t1([&] () {
    runKServer(&svr);
  });
  t1.detach();

  int DUMP_RPC_FD;
  if (FLAGS_dump_rpc) {
    DUMP_RPC_FD = open(FLAGS_dump_rpc_file.c_str(), O_CREAT | O_WRONLY, 00700);
  }

  svr.Post(R"(.*)",
    [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
      std::string body;
      content_reader([&](const char *data, size_t data_length) {
        countBrackets(data, data_length);
        body.append(data, data_length);
        return true;
      });

      write(K_WRITE_FD, body.c_str(), body.length());
      if (FLAGS_dump_rpc) {
        writeWithPrefix(DUMP_RPC_FD, "   > ", body);
      }

      std::string message;
      char buffer[4096] = {0};
      int ret;

      do {
        ret = read(K_READ_FD, buffer, 4096);
        if (ret > 0) message.append(buffer, ret);
      } while (ret > 0 && !doneReading(buffer, ret));

      res.set_content(message, "application/json");
      if (FLAGS_dump_rpc) {
        writeWithPrefix(DUMP_RPC_FD, " <   ", message);
      }
    });

  std::thread t2([&] () {
    svr.listen(FLAGS_host.c_str(), FLAGS_port);
  });

  t2.join();

  if (FLAGS_dump_rpc) {
    close(DUMP_RPC_FD);
  }

  return 0;
}

void runKServer(httplib::Server *svr) {
  int chainId = K_CHAINID;
  bool shutdownable = K_SHUTDOWNABLE, notifications = K_NOTIFICATIONS;
  in_addr address;
  inet_aton("127.0.0.1", &address);

  // injections to KItem for initial configuration variables

  static blockheader injHeaderMode               = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortMode{}, SortKItem{}}"));
  static blockheader injHeaderSchedule           = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortSchedule{}, SortKItem{}}"));
  static blockheader injHeaderInt                = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortInt{}, SortKItem{}}"));
  static blockheader injHeaderBool               = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortBool{}, SortKItem{}}"));
  static blockheader injHeaderEthereumSimulation = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortEthereumSimulation{}, SortKItem{}}"));

  initStaticObjects();
  set_gc_interval(10000);

  // create `Init` configuration variable entries

  static uint64_t mode = (((uint64_t)getTagForSymbolName("LblNORMAL{}")) << 32) | 1;
  inj *modeinj = (inj *)koreAlloc(sizeof(inj));
  modeinj->h = injHeaderMode;
  modeinj->data = (block*)mode;

  uint64_t schedule = (((uint64_t)K_SCHEDULE_TAG) << 32) | 1;
  inj *scheduleinj = (inj *)koreAlloc(sizeof(inj));
  scheduleinj->h = injHeaderSchedule;
  scheduleinj->data = (block*)schedule;

  int input[2], output[2];
  if (pipe(input)) {
    perror("input pipe");
    exit(1);
  }
  if (pipe(output)) {
    perror("output pipe");
    exit(1);
  }

  zinj *inputinj = (zinj *)koreAlloc(sizeof(zinj));
  inputinj->h = injHeaderInt;
  mpz_t input_z;
  mpz_init_set_si(input_z, input[0]);
  inputinj->data = move_int(input_z);
  K_WRITE_FD = input[1];

  zinj *outputinj = (zinj *)koreAlloc(sizeof(zinj));
  outputinj->h = injHeaderInt;
  mpz_t output_z;
  mpz_init_set_si(output_z, output[1]);
  outputinj->data = move_int(output_z);
  K_READ_FD = output[0];

  boolinj *shutdownableinj = (boolinj *)koreAlloc(sizeof(boolinj));
  shutdownableinj->h = injHeaderBool;
  shutdownableinj->data = shutdownable;

  boolinj *notificationsinj = (boolinj *)koreAlloc(sizeof(boolinj));
  notificationsinj->h = injHeaderBool;
  notificationsinj->data = notifications;

  zinj *chaininj = (zinj *)koreAlloc(sizeof(zinj));
  chaininj->h = injHeaderInt;
  mpz_t chainid;
  mpz_init_set_si(chainid, chainId);
  chaininj->data = move_int(chainid);

  static uint64_t accept = (((uint64_t)getTagForSymbolName("Lblaccept{}")) << 32) | 1;
  inj *kinj = (inj *)koreAlloc(sizeof(inj));
  kinj->h = injHeaderEthereumSimulation;
  kinj->data = (block *)accept;

  // build `Init` configuration variable `Map`

  map withSched = hook_MAP_element(configvar("$SCHEDULE"), (block *)scheduleinj);
  map withMode = hook_MAP_update(&withSched, configvar("$MODE"), (block *)modeinj);
  map withInput = hook_MAP_update(&withMode, configvar("$INPUT"), (block *)inputinj);
  map withOutput = hook_MAP_update(&withInput, configvar("$OUTPUT"), (block *)outputinj);
  map withShutdownable = hook_MAP_update(&withOutput, configvar("$SHUTDOWNABLE"), (block *)shutdownableinj);
  map withChain = hook_MAP_update(&withShutdownable, configvar("$CHAINID"), (block *)chaininj);
  map withNotifications = hook_MAP_update(&withChain, configvar("$NOTIFICATIONS"), (block*)notificationsinj);
  map init = hook_MAP_update(&withNotifications, configvar("$PGM"), (block *)kinj);

  // invoke the rewriter
  static uint32_t tag2 = getTagForSymbolName("LblinitGeneratedTopCell{}");
  void *arr[1];
  arr[0] = &init;
  block* init_config = (block *)evaluateFunctionSymbol(tag2, arr);
  block* final_config = take_steps(K_DEPTH, init_config);
  if (FLAGS_dump) printConfiguration("/dev/stderr", final_config);
  close(K_WRITE_FD);
  close(K_READ_FD);
  svr->stop();
}

void bracketHelper(char c) {
  switch(c){
    case '{':{
      brace_counter_++;
      break;
    }
    case '}':{
      brace_counter_--;
      break;
    }
    case '[':{
      bracket_counter_++;
      break;
    }
    case ']':{
      bracket_counter_--;
      break;
    }
  }
}

void countBrackets(const char *buffer, size_t len) {
  for(int i = 0; i < len; i++) {
    bracketHelper(buffer[i]);
    if(0 == brace_counter_ && 0 == bracket_counter_) {
      object_counter_++;
    }
  }
}

bool doneReading (const char *buffer, int len) {
  for(int i = 0; i < len; i++){
    bracketHelper(buffer[i]);
    if(0 == brace_counter_ && 0 == bracket_counter_){
      object_counter_--;
    }
  }
  return 0 == brace_counter_
      && 0 == bracket_counter_
      && 0 == object_counter_;
}

void writeWithPrefix(int fd, std::string prefix, std::string msg) {
  std::string msgNew = prefix + std::regex_replace(msg, std::regex("\n"), "\n" + prefix);
  write(fd, msgNew.c_str(), msgNew.length());
}

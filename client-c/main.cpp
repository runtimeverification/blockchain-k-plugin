#include <iostream>
#include <regex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <httplib.h>
#include "runtime/alloc.h"
#include "version.h"
#include "init.h"
#include <gflags/gflags.h>

void runKServer();
void openSocket();

static bool K_SHUTDOWNABLE;
static uint32_t K_SCHEDULE_TAG;
static int K_CHAINID;
static int K_PORT = 9191;
static int K_SOCKET;
static int K_DEPTH;

static std::string FRONTIER = "frontier";
static std::string HOMESTEAD = "homestead";
static std::string TANGERINE_WHISTLE = "tangerine_whistle";
static std::string SPURIOUS_DRAGON = "spurious_dragon";
static std::string BYZANTIUM = "byzantium";
static std::string CONSTANTINOPLE = "constantinople";
static std::string PETERSBURG = "petersburg";

DEFINE_int32(port, 8545, "Port to listen on with HTTP protocol");
DEFINE_int32(kport, 9191, "The port on which the connection between the HTTP Server and the K Server is made");
DEFINE_int32(depth, -1, "For debugging, stop execution at a certain depth.");
DEFINE_string(host, "localhost", "IP/Hostname to bind to");
DEFINE_bool(shutdownable, false, "Allow `firefly_shutdown` message to kill server");
DEFINE_string(hardfork, "petersburg", "Ethereum client hardfork. Supported: 'frontier', "
             "'homestead', 'tangerine_whistle', 'spurious_dragon', 'byzantium', "
             "'constantinople', 'petersburg'");
DEFINE_int32(networkId, 28346, "Set network chain id");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_bool(vmversion, false, "Display current VM version");

int brace_counter_, bracket_counter_, object_counter_;

void countBrackets(const char *buffer) {
  for(int i = 0; i < strlen(buffer); i++) {
    switch(buffer[i]){
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
    if(0 == brace_counter_ && 0 == bracket_counter_) {
      object_counter_++;
    }
  }
}

bool doneReading(const char *buffer) {
  countBrackets(buffer);
  return 0 == brace_counter_
      && 0 == bracket_counter_
      && 0 == object_counter_;
}

int main(int argc, char **argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  K_SCHEDULE_TAG = getTagForSymbolName("LblPETERSBURG'Unds'EVM{}");

  if (FLAGS_vmversion) {
    std::cout << argv[0] << " version " << VM_VERSION << std::endl;
    return 0;
  }

  K_CHAINID = FLAGS_networkId;
  K_SHUTDOWNABLE = FLAGS_shutdownable;
  K_PORT = FLAGS_kport;
  K_DEPTH = FLAGS_depth;

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
  } else {
      std::cerr << "Invalid hardfork found: " << FLAGS_hardfork << std::endl;
      return 1;
  }

  // Start KServer in a separate thread
  std::thread t1([&] () {
    runKServer();
  });
  t1.detach();

  openSocket();

  // Start HTTPServer mainloop in a separate thread
  //std::thread t2([&] () {
  //  server.start();
  //});

  httplib::Server svr;

  svr.Post(R"(.*)",
    [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
      std::string body;
      content_reader([&](const char *data, size_t data_length) {
        body.append(data, data_length);
        return true;
      });
      send(K_PORT, body.c_str(), body.length(), 0);

      std::string message;
      char buffer[4096] = {0};
      int ret;

      do {
        ret = recv(K_PORT, buffer, 4096, 0);
        buffer[ret] = 0x00;
        message += buffer;
      } while (ret > 0 && !doneReading(buffer));

      // Need to read actual response and send it over
      res.set_content(message, "application/json");
    });

  // need to stop the server at some point with svr.stop()
  svr.listen(FLAGS_ip.c_str(), FLAGS_port);

  //t2.join();
  //t1.join();
  return 0;
}

void runKServer() {
  int port = K_PORT, chainId = K_CHAINID, shutdownable = K_SHUTDOWNABLE;
  in_addr address;
  inet_aton("127.0.0.1", &address);

  // injections to KItem for initial configuration variables

  static blockheader injHeaderMode               = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortMode{}, SortKItem{}}"));
  static blockheader injHeaderSchedule           = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortSchedule{}, SortKItem{}}"));
  static blockheader injHeaderInt                = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortInt{}, SortKItem{}}"));
  static blockheader injHeaderBool               = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortBool{}, SortKItem{}}"));
  static blockheader injHeaderEthereumSimulation = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortEthereumSimulation{}, SortKItem{}}"));

  // create `Init` configuration variable entries

  static uint64_t mode = (((uint64_t)getTagForSymbolName("LblNORMAL{}")) << 32) | 1;
  inj *modeinj = (inj *)koreAlloc(sizeof(inj));
  modeinj->h = injHeaderMode;
  modeinj->data = (block*)mode;

  uint64_t schedule = (((uint64_t)K_SCHEDULE_TAG) << 32) | 1;
  inj *scheduleinj = (inj *)koreAlloc(sizeof(inj));
  scheduleinj->h = injHeaderSchedule;
  scheduleinj->data = (block*)schedule;

  int sock = init(port, address);
  zinj *sockinj = (zinj *)koreAlloc(sizeof(zinj));
  sockinj->h = injHeaderInt;
  mpz_t sock_z;
  mpz_init_set_si(sock_z, sock);
  sockinj->data = move_int(sock_z);

  boolinj *shutdownableinj = (boolinj *)koreAlloc(sizeof(boolinj));
  shutdownableinj->h = injHeaderBool;
  shutdownableinj->data = shutdownable;

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
  map withSocket = hook_MAP_update(&withMode, configvar("$SOCK"), (block *)sockinj);
  map withShutdownable = hook_MAP_update(&withSocket, configvar("$SHUTDOWNABLE"), (block *)shutdownableinj);
  map withChain = hook_MAP_update(&withShutdownable, configvar("$CHAINID"), (block *)chaininj);
  map init = hook_MAP_update(&withChain, configvar("$PGM"), (block *)kinj);

  // invoke the rewriter
  static uint32_t tag2 = getTagForSymbolName("LblinitGeneratedTopCell{}");
  void *arr[1];
  arr[0] = &init;
  std::cout << "Starting K Server on port " << K_PORT << std::endl;
  block* init_config = (block *)evaluateFunctionSymbol(tag2, arr);
  block* final_config = take_steps(K_DEPTH, init_config);
  printConfiguration("/dev/stderr", final_config);
  shutdown(K_SOCKET, SHUT_RDWR);
  //server->stop();
}

void openSocket() {
  struct sockaddr_in k_addr;
  int sec = 0;
  int ret = -1;
  if ((K_SOCKET = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
      std::cerr << "\n Socket creation error \n";
      return;
  }

  k_addr.sin_family = AF_INET;
  k_addr.sin_port = htons(K_PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &k_addr.sin_addr) <= 0)
  {
    std::cerr << "\nInvalid address/ Address not supported \n";
    return;
  }
  while (ret != 0 && sec < 4) {
    sleep(sec);
    ret = connect(K_SOCKET, (struct sockaddr *)&k_addr, sizeof(k_addr));
    sec++;
  }
  if (ret != 0) {
    std::cerr << "Socket connection to K Server failed, tried " << sec << " times." << std::endl;
  }
  std::cout << "Socket connection to K Server is open\n";
}

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include "runtime/alloc.h"
#include "version.h"
#include "init.h"

static std::string FRONTIER = "frontier";
static std::string HOMESTEAD = "homestead";
static std::string TANGERINE_WHISTLE = "tangerine_whistle";
static std::string SPURIOUS_DRAGON = "spurious_dragon";
static std::string BYZANTIUM = "byzantium";
static std::string CONSTANTINOPLE = "constantinople";
static std::string PETERSBURG = "petersburg";

int main(int argc, char **argv) {
  std::string usage = std::string("Usage: ") + argv[0] + " [OPTIONS] [-p PORT] [-h HOST]\n"
	  "A KEVM-powered in-memory Web3 Ethereum client by Runtime Verification\n"
	  "\n"
	  "Network:\n"
	  "  -h,--host=IP        Bind server to IP\n"
	  "  -p,--port=PORT      Listen to requests on port PORT\n"
	  "\n"
	  "Chain:\n"
	  "  -k,--hardfork=FORK  Ethereum client implements hardfork FORK;\n"
          "                      FORK is 'frontier', 'homestead', 'tangerine_whistle',\n"
          "                      'spurious_dragon', 'byzantium', 'constantinople',\n"
          "                      or 'petersburg'\n"
	  "  -i,--networkId=ID   Set network chain id to ID\n";
  int flag, port = 8545, chainId = 28346;
  in_addr address;
  inet_aton("127.0.0.1", &address);
  int help = false, version = false;
  uint32_t schedule_tag = getTagForSymbolName("LblPETERSBURG'Unds'EVM{}");
  while(1) {
    static struct option long_options[] = {
      {"help", no_argument, &help, true},
      {"verison", no_argument, &version, true},
      {"host", required_argument, 0, 'h'},
      {"hostname", required_argument, 0, 'h'},
      {"port", required_argument, 0, 'p'},
      {"hardfork", required_argument, 0, 'k'},
      {"networkId", required_argument, 0, 'i'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    flag = getopt_long(argc, argv, "h:p:k:i:", long_options, &option_index);
    if (flag == -1) {
      break;
    }
    switch(flag) {
    case 0:
      break;
    case 'h':
      if (!inet_aton(optarg, &address)) {
        std::cerr << "Invalid bind address" << std::endl;
        return 1;
      }
      break;
    case 'p':
      port = std::stoi(optarg);
      break;
    case 'k':
      if (optarg == FRONTIER) {
        schedule_tag = getTagForSymbolName("LblFRONTIER'Unds'EVM{}");
      } else if (optarg == HOMESTEAD) {
        schedule_tag = getTagForSymbolName("LblHOMESTEAD'Unds'EVM{}");
      } else if (optarg == TANGERINE_WHISTLE) {
        schedule_tag = getTagForSymbolName("LblTANGERINE'Unds'WHISTLE'Unds'EVM{}");
      } else if (optarg == SPURIOUS_DRAGON) {
        schedule_tag = getTagForSymbolName("LblSPURIOUS'Unds'DRAGON'Unds'EVM{}");
      } else if (optarg == BYZANTIUM) {
        schedule_tag = getTagForSymbolName("LblBYZANTIUM'Unds'EVM{}");
      } else if (optarg == CONSTANTINOPLE) {
        schedule_tag = getTagForSymbolName("LblCONSTANTINOPLE'Unds'EVM{}");
      } else if (optarg == PETERSBURG) {
        schedule_tag = getTagForSymbolName("LblPETERSBURG'Unds'EVM{}");
      } else {
	std::cerr << "Invalid hardfork found: " << optarg << std::endl;
	return 1;
      }
      break;
    case 'i':
      chainId = std::stoi(optarg);
      break;
    default:
      std::cerr << "Invalid option: " << (char)flag << std::endl;
      return 1;
    }
  }
  if (help) {
    std::cout << usage;
    return 0;
  } else if (version) {
    std::cout << argv[0] << " version " << VM_VERSION << std::endl;
    return 0;
  }

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

  uint64_t schedule = (((uint64_t)schedule_tag) << 32) | 1;
  inj *scheduleinj = (inj *)koreAlloc(sizeof(inj));
  scheduleinj->h = injHeaderSchedule;
  scheduleinj->data = (block*)schedule;

  int sock = init(port, address);
  zinj *sockinj = (zinj *)koreAlloc(sizeof(zinj));
  sockinj->h = injHeaderInt;
  mpz_t sock_z;
  mpz_init_set_si(sock_z, sock);
  sockinj->data = move_int(sock_z);

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
  map withChain = hook_MAP_update(&withSocket, configvar("$CHAINID"), (block *)chaininj);
  map init = hook_MAP_update(&withChain, configvar("$PGM"), (block *)kinj);

  // invoke the rewriter

  static uint32_t tag2 = getTagForSymbolName("LblinitGeneratedTopCell{}");
  void *arr[1];
  arr[0] = &init;
  block* init_config = (block *)evaluateFunctionSymbol(tag2, arr);
  block* final_config = take_steps(-1, init_config);
  printConfiguration("/dev/stderr", final_config);
}


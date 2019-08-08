#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gmp.h>
#include "runtime/alloc.h"
#include "version.h"

extern "C" {
  void setKoreMemoryFunctionsForGMP(void);
  extern thread_local uint64_t INTERVAL;
}

int init(int port, in_addr host);


int main(int argc, char **argv) {
  std::string usage = std::string("Usage: ") + argv[0] + " PORT BIND_ADDRESS";
  if (argc == 2 && argv[1] == std::string("--help")) {
    std::cout << usage << std::endl;
    return 0;
  } else if (argc == 2 && argv[1] == std::string("--version")) {
    std::cout << argv[0] << " version " << VM_VERSION << std::endl;
    return 0;
  } else if (argc != 3) {
    std::cerr << "Incorrect number of arguments" << std::endl;
    std::cerr << usage << std::endl;
    return 1;
  }
  int port = std::stoi(argv[1]);
  in_addr host;
  if (!inet_aton(argv[2], &host)) {
    std::cerr << "Invalid bind address" << std::endl;
    return 1;
  }

  int sock = init(port, host);
}

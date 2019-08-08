int init(int port, in_addr host) {
  setKoreMemoryFunctionsForGMP();

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = host;
  int sec = 0;
  int ret;
  do {
    if (sec > 0) {
      std::cerr << "Socket in use, retrying in " << sec << "..." << std::endl;
    }
    sleep(sec);
    ret = bind(sock, (sockaddr *)&addr, sizeof(addr));
    sec++;
  } while(ret == -1 && errno == EADDRINUSE);
  if (ret) {
    perror("bind");
    return 1;
  }
  if (listen(sock, 50)) {
    perror("listen");
    return 1;
  }
  INTERVAL = 10000;
  return sock;
}

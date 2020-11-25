#pragma once

#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <linux/tcp.h>
#include <unistd.h>

[[noreturn]] void serve(int port, void (*callback)(int)) {
  const static int ON = 1;
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{AF_INET, htons(port), {}, {}};
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &ON, sizeof(ON));
  struct sigaction act {};
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, nullptr);
  signal(SIGPIPE, [](int) {});
  bind(listenfd, (sockaddr *)&addr, sizeof(addr));
  listen(listenfd, 20);
  while (true) {
    int connfd = accept(listenfd, nullptr, nullptr);
    if (connfd > 0) {
      setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &ON, sizeof(ON));
      callback(connfd);
      close(connfd);
    }
  }
}
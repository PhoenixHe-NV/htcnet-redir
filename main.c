#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "session.h"
#include "server.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stdout, "Usage: %s <PORT>", argv[0]);
    exit(-1);
  }
  int port = (int) strtol(argv[1], NULL, 10);

  signal(SIGPIPE, SIG_IGN);

  uv_loop_t* loop = uv_default_loop();
  uv_tcp_t server;

  session_init(loop);

  if (server_listen_init(loop, &server, port)) {
    return -1;
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}
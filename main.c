#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "session.h"
#include "server.h"

int64_t counter = 0;

void wait_for_a_while(uv_idle_t* handle) {
  counter++;

//  if (counter >= 10e6) {
//    uv_idle_stop(handle);
//  }
}


int main() {

  session_init();

  uv_loop_t* loop = uv_default_loop();
  uv_tcp_t server;

  if (server_listen_init(loop, &server, 9007)) {
    return -1;
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}
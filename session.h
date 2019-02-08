//
// Created by Phoenix He on 2019-02-08.
//

#ifndef HTCNET_REDIR_SESSION_H
#define HTCNET_REDIR_SESSION_H

#include <time.h>
#include <uv.h>

typedef struct session {
  struct session* prev;
  struct session* next;
  time_t lastUpdate;

  uv_tcp_t client;
  uv_tcp_t remote;
} session_t;

void session_init();
void session_touch(session_t *current);


#endif //HTCNET_REDIR_SESSION_H

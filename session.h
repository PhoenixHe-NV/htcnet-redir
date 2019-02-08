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
  uv_connect_t req;
} session_t;

void session_init();
void session_touch(session_t *current);
session_t* session_create(uv_loop_t* loop);
void session_end(session_t* current);
void session_clear_timeout(time_t timeout);

#endif //HTCNET_REDIR_SESSION_H

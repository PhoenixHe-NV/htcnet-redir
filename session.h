//
// Created by Phoenix He on 2019-02-08.
//

#ifndef HTCNET_REDIR_SESSION_H
#define HTCNET_REDIR_SESSION_H

#include <time.h>
#include <uv.h>
#include "buf.h"


typedef struct session_copy_s {
  uv_tcp_t uv_tcp;
  uv_write_t pair_write_req;
  uv_buf_t uv_buf;
  buf_ref copy_buf_ref;
} session_copy_t;

typedef struct session_s {
  struct session_s* prev;
  struct session_s* next;
  time_t lastUpdate;

  uv_connect_t remote_connect_req;
  session_copy_t client, remote;
} session_t;

#define SESSION_PAIR(session, session_copy) \
  (&session->client == session_copy ? &session->remote  : &session->client )

void session_init(uv_loop_t* loop);
void session_touch(session_t *current);
session_t* session_create(uv_loop_t* loop);
void session_end(session_t* current);
void session_clear_timeout(time_t timeout);
uint32_t get_session_count();

#endif //HTCNET_REDIR_SESSION_H

//
// Created by Phoenix He on 2019-02-08.
//

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"
#include "session.h"

static session_t active_list;

static void init_list(session_t* session_list) {
  session_list->next = session_list;
  session_list->prev = session_list;
}

static void add_session_to_tail(session_t* session_list, session_t* current) {
  session_t* prev = session_list->prev;

  prev->next = current;
  current->prev = prev;

  current->next = session_list;
  session_list->prev = current;
}

static void remove_session_from_list(session_t* current) {
  session_t* prev = current->prev;
  session_t* next = current->next;

  prev->next = next;
  next->prev = prev;

  current->next = NULL;
  current->prev= NULL;
}

static void set_session_last_update(session_t* current) {
  time(&current->lastUpdate);
}

static void close_conn_cb(uv_handle_t* uv_tcp) {
  uv_unref(uv_tcp);
  session_t* session = uv_tcp->data;

  if (uv_has_ref((uv_handle_t *) &session->client) == 0
    && uv_has_ref((uv_handle_t*) &session->remote) == 0) {
    // All closed, we can free our session
    free(session);

    fprintf(stderr, "%p: Session closed", session);
  }
}

static void close_conn_cb_on_create1(uv_handle_t* uv_tcp) {
  session_t* session = uv_tcp->data;
  free(session);
}

void session_touch(session_t* current) {
  remove_session_from_list(current);
  set_session_last_update(current);
  add_session_to_tail(&active_list, current);
}


session_t* session_create(uv_loop_t* loop) {
  session_t* current = malloc(sizeof(session_t));
  if (current == NULL) {
    perror("malloc");
    return NULL;
  }

  memset(current, 0, sizeof(session_t));

  current->client.data = current;
  current->remote.data = current;
  current->req.data = current;

  UV_CHECK_GOTO("uv_tcp_init client", session_create_error, uv_tcp_init(loop, &current->client));
  UV_CHECK_GOTO("uv_tcp_init remote", session_create_error1, uv_tcp_init(loop, &current->remote));

  set_session_last_update(current);
  add_session_to_tail(&active_list, current);

  return current;

session_create_error1:
  uv_close((uv_handle_t *) &current->client, close_conn_cb_on_create1);
  return NULL;

session_create_error:
  free(current);
  return NULL;
}

void session_end(session_t* current) {
  remove_session_from_list(current);

  uv_ref((uv_handle_t *) &current->client);
  uv_ref((uv_handle_t *) &current->remote);

  uv_close((uv_handle_t *) &current->client, close_conn_cb);
  uv_close((uv_handle_t *) &current->remote, close_conn_cb);
}

void session_clear_timeout(time_t timeout) {
  time_t threshold;
  time(&threshold);
  threshold = threshold - timeout;

  while (true) {
    session_t* head = active_list.next;
    if (head == &active_list || head->lastUpdate > threshold) {
      return;
    }
    remove_session_from_list(head);
  }
}

void session_init() {
  init_list(&active_list);
}

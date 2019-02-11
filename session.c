//
// Created by Phoenix He on 2019-02-08.
//

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"
#include "session.h"

static session_t active_list;

static uint32_t session_count;

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

uint32_t get_session_count() {
  return session_count;
}

session_t* session_create(uv_loop_t* loop) {
  session_t* current = malloc(sizeof(session_t));
  if (current == NULL) {
    perror("malloc");
    return NULL;
  }

  memset(current, 0, sizeof(session_t));

  current->client.uv_tcp.data = current;
  current->remote.uv_tcp.data = current;
  current->remote_connect_req.data = current;

  UV_CHECK_GOTO("uv_tcp_init client", session_create_error, uv_tcp_init(loop, &current->client.uv_tcp));
  UV_CHECK_GOTO("uv_tcp_init remote", session_create_error1, uv_tcp_init(loop, &current->remote.uv_tcp));

  set_session_last_update(current);
  add_session_to_tail(&active_list, current);
  session_count++;

  return current;

session_create_error1:
  uv_close((uv_handle_t *) &current->client, close_conn_cb_on_create1);
  return NULL;

session_create_error:
  free(current);
  return NULL;
}

void session_end(session_t* current) {
  if (current->prev == NULL && current->next == NULL) {
    // Disposing
    return;
  }

  buf_free(&current->client.copy_buf_ref);
  buf_free(&current->remote.copy_buf_ref);

  remove_session_from_list(current);

  uv_ref((uv_handle_t *) &current->client);
  uv_ref((uv_handle_t *) &current->remote);

  uv_close((uv_handle_t *) &current->client, close_conn_cb);
  uv_close((uv_handle_t *) &current->remote, close_conn_cb);

  session_count--;
  fprintf(stderr, "%p: - C:%d\n", current, session_count);
}

void session_clear_timeout(time_t timeout) {
  time_t threshold;
  time(&threshold);
  threshold = threshold - timeout;
  bool cleared = 0;

  while (true) {
    session_t* head = active_list.next;
    if (head == &active_list || head->lastUpdate > threshold) {
      goto end;
    }
    if (!cleared) {
      cleared = 1;
      fprintf(stderr, "--- Timeout Clear Start --- \n");
    }

    session_end(head);
  }

end:
  if (cleared) {
    fprintf(stderr, "--- Timeout Clear Stop --- \n");
  }
}

static uv_check_t session_check;

static void session_check_cb(uv_check_t* handle) {
  session_clear_timeout(600);
}

void session_init(uv_loop_t* loop) {
  init_list(&active_list);

  session_count = 0;

  uv_check_init(loop, &session_check);
  uv_check_start(&session_check, session_check_cb);
}

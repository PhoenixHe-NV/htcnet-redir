//
// Created by Phoenix He on 2019-02-08.
//

#include "session.h"

static session_t session_list;

static void add_session_to_tail(session_t* current) {
  session_t* prev = session_list.prev;

  prev->next = current;
  current->prev = prev;

  current->next = &session_list;
  session_list.prev = current;
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

void session_touch(session_t *current) {
  remove_session_from_list(current);
  set_session_last_update(current);
  add_session_to_tail(current);
}

void session_init() {
  session_list.next = &session_list;
  session_list.prev = &session_list;
}



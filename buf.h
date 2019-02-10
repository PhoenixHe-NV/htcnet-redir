//
// Created by Phoenix He on 2019-02-10.
//

#ifndef HTCNET_REDIR_BUF_H
#define HTCNET_REDIR_BUF_H

#include <stdint-gcc.h>

#define BUF_ALIGNMENT 16

typedef struct buf_block_s {
  struct buf_block_s* next;
  size_t ref_count;
  size_t block_size;
  size_t cur_end;

  struct {} __attribute__ ((aligned (BUF_ALIGNMENT)));
  char data[];
} buf_block;

typedef struct buf_ref_s {
  size_t size;
  char* data;
  buf_block* buf_block;
} buf_ref;

void buf_alloc(size_t suggested_size, buf_ref* ref);
void buf_free(buf_ref* ref);

#endif //HTCNET_REDIR_BUF_H

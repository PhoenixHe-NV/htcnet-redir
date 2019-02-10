//
// Created by Phoenix He on 2019-02-10.
//


#include <stdlib.h>
#include <stdio.h>

#include "buf.h"

#define BUF_BLOCK_SIZE 4*1024*1024
#define BUF_BLOCK_FREE_THRESHOLD 8

buf_block* buf_block_free_head = NULL;
size_t buf_block_free_count = 0;

buf_block* buf_block_cur = NULL;

static buf_block* buf_block_alloc(size_t suggested_size) {
  buf_block* block;

  if (buf_block_free_head != NULL) {
    block = buf_block_free_head;
    buf_block_free_head = block->next;
    --buf_block_free_count;

  } else {
    block = malloc(sizeof(buf_block) + suggested_size);
    block->block_size = suggested_size;
    fprintf(stderr, "buf_block_alloc %p\n", block);
  }

  block->ref_count = 0;
  block->cur_end = 0;
  block->next = NULL;

  return block;
}

static void buf_block_free(buf_block* block) {
  if (buf_block_free_count >= BUF_BLOCK_FREE_THRESHOLD) {
    fprintf(stderr, "buf_block_free %p\n", block);
    free(block);
    return;
  }

  block->next = buf_block_free_head;
  buf_block_free_head = block;
  buf_block_free_count++;
}

static buf_block* buf_block_cur_get() {
  if (buf_block_cur == NULL) {
    buf_block_cur = buf_block_alloc(BUF_BLOCK_SIZE);
  }

  return buf_block_cur;
}

static buf_block* buf_block_cur_realloc() {
  buf_block_cur = buf_block_alloc(BUF_BLOCK_SIZE);
  return buf_block_cur;
}

void buf_alloc(size_t suggested_size, buf_ref* ref) {
  size_t size = ((suggested_size - 1)/BUF_ALIGNMENT + 1) * BUF_ALIGNMENT;

  if (size > BUF_BLOCK_SIZE) {
    size = BUF_BLOCK_SIZE;
  }

  buf_block* block = buf_block_cur_get();
  if (block->block_size - block->cur_end < size) {
    block = buf_block_cur_realloc();
  }

  ref->buf_block = block;
  ref->data = ((char*) &block->data) + block->cur_end;
  ref->size = size;
  block->cur_end += size;
  block->ref_count++;
}

void buf_free(buf_ref* ref) {
  buf_block* block = ref->buf_block;
  if (block == NULL) {
    return;
  }

  block->ref_count--;
  if (block->ref_count == 0) {
    if (block == buf_block_cur) {
      buf_block_cur = NULL;
    }
    buf_block_free(block);
  }

  ref->buf_block = NULL;
  ref->data = NULL;
  ref->size = 0;
}


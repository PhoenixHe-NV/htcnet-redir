//
// Created by Phoenix He on 2019-02-08.
//

#include <stdlib.h>
#include <uv.h>
#include <memory.h>
#include <linux/netfilter_ipv4.h>

#include "utils.h"
#include "session.h"
#include "server.h"
#include "buf.h"

static void new_conn_log(uv_tcp_t* client, struct sockaddr_storage* orig_dst, struct sockaddr_storage* orig_src) {
  char orig_dst_str[INET6_ADDRSTRLEN], orig_src_str[INET6_ADDRSTRLEN];
  get_addr_str(orig_dst, orig_dst_str);
  get_addr_str(orig_src, orig_src_str);
  fprintf(stderr, "%p: + C:%d %s:%d to %s:%d\n",
    client->data, get_session_count(), orig_src_str, get_addr_port(orig_src), orig_dst_str, get_addr_port(orig_dst));
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  session_copy_t* session_copy = (session_copy_t *) handle;

  buf_alloc(suggested_size, &session_copy->copy_buf_ref);
  buf->base = session_copy->copy_buf_ref.data;
  buf->len = session_copy->copy_buf_ref.size;
}

static void write_cb(uv_write_t* req, int status);

static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread == 0) {
    return;
  }

  session_t* session = stream->data;
  if (nread < 0) {
    // EOF or Error
    session_end(session);
    return;
  }
  session_touch(session);

  uv_read_stop(stream);

  session_copy_t* session_copy = (session_copy_t *) stream;
  session_copy_t* pair = SESSION_PAIR(session, session_copy);
  session_copy->uv_buf.base = buf->base;
  session_copy->uv_buf.len = (size_t) nread;
  uv_write(&session_copy->pair_write_req, (uv_stream_t *) &pair->uv_tcp, &session_copy->uv_buf, 1, write_cb);
}

static void write_cb(uv_write_t* req, int status) {
  session_copy_t* pair = (session_copy_t *) req->handle;
  session_t* session = pair->uv_tcp.data;
  session_copy_t* session_copy = SESSION_PAIR(session, pair);

  buf_free(&session_copy->copy_buf_ref);

  if (status < 0) {
    // Error
    session_end(session);
    return;
  }
  session_touch(session);

  uv_read_start((uv_stream_t *) &session_copy->uv_tcp, alloc_cb, read_cb);
}

static void remote_connect_cb(uv_connect_t* req, int status) {
  session_t* session = req->data;

  if (status != 0) {
    fprintf(stderr, "%p: Failed to connect\n", session);
    session_end(session);
    return;
  }

  uv_tcp_nodelay(&session->client.uv_tcp, 1);
  uv_tcp_nodelay(&session->remote.uv_tcp, 1);

  session_touch(session);

  uv_read_start((uv_stream_t*) &session->client.uv_tcp, alloc_cb, read_cb);
  uv_read_start((uv_stream_t*) &session->remote.uv_tcp, alloc_cb, read_cb);
}

static void new_connection_cb(uv_stream_t* server, int status) {
  if (status) {
    uv_print_error("new_connection status", status);
    return;
  }

  session_t* session = session_create(uv_default_loop());
  if (session == NULL) {
    return;
  }

  uv_tcp_t* client = &session->client.uv_tcp;
  uv_tcp_t* remote = &session->remote.uv_tcp;

  // Accept new connection
  UV_CHECK_GOTO("uv_accept", close_conn, uv_accept(server, (uv_stream_t *) client));

  int local_fd = uv_get_fd((uv_handle_t *) client);

  struct sockaddr_storage orig_dst, orig_src;
  SYS_CHECK_GOTO("get original dst addr", close_conn, get_org_dst_addr(local_fd, &orig_dst));

  int orig_src_len;
  UV_CHECK_GOTO("uv_tcp_getpeername", close_conn, uv_tcp_getpeername(client, (struct sockaddr *) &orig_src, &orig_src_len));

  new_conn_log(client, &orig_dst, &orig_src);

  // Connect to remote
  uv_tcp_connect(&session->remote_connect_req, remote, (const struct sockaddr *) &orig_dst, remote_connect_cb);

  return;

close_conn:
  session_end(session);
}

int server_listen_init(uv_loop_t *loop, uv_tcp_t *server, int port) {
  struct sockaddr_in bind_addr;
  UV_CHECK("uv_ip4_addr", uv_ip4_addr("0.0.0.0", port, &bind_addr));

  UV_CHECK("uv_tcp_init", uv_tcp_init(loop, server));

  UV_CHECK("uv_tcp_bind", uv_tcp_bind(server, (struct sockaddr *) &bind_addr, 0));

  UV_CHECK("uv_listen", uv_listen((uv_stream_t*) server, 30, new_connection_cb));

  return 0;
}

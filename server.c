//
// Created by Phoenix He on 2019-02-08.
//

#include <stdlib.h>
#include <uv.h>
#include <memory.h>
#include <linux/netfilter_ipv4.h>

#include "utils.h"
#include "server.h"


static void new_conn_log(uv_tcp_t* client, struct sockaddr_storage* orig_dst, struct sockaddr_storage* orig_src) {
  char orig_dst_str[INET6_ADDRSTRLEN], orig_src_str[INET6_ADDRSTRLEN];
  get_addr_str(orig_dst, orig_dst_str);
  get_addr_str(orig_src, orig_src_str);
  fprintf(stderr, "%p: Accept new connection from %s:%d to %s:%d\n",
    client, orig_src_str, get_addr_port(orig_src), orig_dst_str, get_addr_port(orig_dst));
}

static void close_conn_cb(uv_handle_t* client) {
  fprintf(stderr, "%p: Connection closed\n", client);
  free(client);
}

static void new_connection_cb(uv_stream_t* server, int status) {
  if (status) {
    uv_print_error("new_connection status", status);
    return;
  }

  uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
  UV_CHECK_VOID("uv_tcp_init client", uv_tcp_init(uv_default_loop(), client));

  // Accept new connection
  UV_CHECK_GOTO("uv_accept", close_conn, uv_accept(server, (uv_stream_t *) client));

  int local_fd = uv_get_fd((uv_handle_t *) client);

  struct sockaddr_storage orig_dst, orig_src;
  SYS_CHECK_GOTO("get original dst addr", close_conn, get_org_dst_addr(local_fd, &orig_dst));

  int orig_src_len;
  UV_CHECK_GOTO("uv_tcp_getpeername", close_conn, uv_tcp_getpeername(client, (struct sockaddr *) &orig_src, &orig_src_len));

  new_conn_log(client, &orig_dst, &orig_src);

  // Connect to remote
  uv_connect_t* connect = malloc(sizeof(uv_connect_t));
  UV_CHECK_VOID("uv_tcp_init client", uv_tcp_init(uv_default_loop(), client));

close_conn:
  uv_close((uv_handle_t*) client, close_conn_cb);
}

int server_listen_init(uv_loop_t *loop, uv_tcp_t *server, int port) {
  struct sockaddr_in bind_addr;
  UV_CHECK("uv_ip4_addr", uv_ip4_addr("0.0.0.0", port, &bind_addr));

  UV_CHECK("uv_tcp_init", uv_tcp_init(loop, server));

  UV_CHECK("uv_tcp_bind", uv_tcp_bind(server, (struct sockaddr *) &bind_addr, 0));

  UV_CHECK("uv_listen", uv_listen((uv_stream_t*) server, 30, new_connection_cb));
}

//
// Created by Phoenix He on 2019-02-08.
//

#ifndef HTCNET_REDIR_SERVER_H
#define HTCNET_REDIR_SERVER_H

#include <uv.h>

int server_listen_init(uv_loop_t *loop, uv_tcp_t *server, int port);

#endif //HTCNET_REDIR_SERVER_H

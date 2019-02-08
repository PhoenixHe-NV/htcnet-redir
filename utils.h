//
// Created by Phoenix He on 2019-02-08.
//

#ifndef HTCNET_REDIR_UTILS_H
#define HTCNET_REDIR_UTILS_H

#include <errno.h>

int uv_print_error(char* desc, int code);
int uv_get_fd(uv_handle_t* handle);

int get_addr_port(struct sockaddr_storage *addr);
void get_addr_str(struct sockaddr_storage *addr, char *addr_str);
int get_org_dst_addr(int sock_fd, struct sockaddr_storage *orig_dst);

#define UV_CHECK(desc, func_call) { \
  int ret = func_call;              \
  if (ret) {                        \
    uv_print_error(desc, ret);      \
    return ret;                     \
  }                                 \
}

#define UV_CHECK_VOID(desc, func_call) { \
  int ret = func_call;              \
  if (ret) {                        \
    uv_print_error(desc, ret);      \
    return;                         \
  }                                 \
}

#define UV_CHECK_GOTO(desc, label, func_call) { \
  int ret = func_call;              \
  if (ret) {                        \
    uv_print_error(desc, ret);      \
    goto label;                     \
  }                                 \
}

#define SYS_CHECK_VOID(desc, func_call) { \
  if (func_call) {                  \
    uv_print_error(desc, uv_translate_sys_error(errno));      \
    return;                         \
  }                                 \
}

#define SYS_CHECK_GOTO(desc, label, func_call) { \
  if (func_call) {                  \
    uv_print_error(desc, uv_translate_sys_error(errno));      \
    goto label;                         \
  }                                 \
}


#endif //HTCNET_REDIR_UTILS_H

//
// Created by Phoenix He on 2019-02-08.
//

#include <uv.h>
#include <string.h>
#include <linux/netfilter_ipv4.h>

#include "utils.h"

int uv_print_error(char* desc, int code) {
  fprintf(stderr, "Error: %s: %s\n", desc, uv_err_name(code));
  return 0;
}

int uv_get_fd(uv_handle_t* t) {
  uv_os_fd_t local_fd;
  int ret = uv_fileno(t, &local_fd);
  if (ret) {
    uv_print_error("uv_get_fd", ret);
    return -1;
  }

  return local_fd;
}

int get_org_dst_addr(int sock_fd, struct sockaddr_in* orig_dst){
  socklen_t addr_len = sizeof(struct sockaddr_in);

  memset(orig_dst, 0, addr_len);

  //For UDP transparent proxying:
  //Set IP_RECVORIGDSTADDR socket option for getting the original
  //destination of a datagram

  //Socket is bound to original destination
  if (getsockopt(sock_fd, SOL_IP, SO_ORIGINAL_DST, orig_dst, &addr_len) == 0) {
    goto GET_ORG_DSTADDR_SUCCESS;
  }

  if (getsockname(sock_fd, (struct sockaddr*) orig_dst, &addr_len) == 0 ) {
    goto GET_ORG_DSTADDR_SUCCESS;
  }

  return -1;

    GET_ORG_DSTADDR_SUCCESS:
  return 0;
}

int get_addr_port(struct sockaddr_in* addr) {
  return ntohs(addr->sin_port);
}

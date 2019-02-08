//
// Created by Phoenix He on 2019-02-08.
//

#include <uv.h>
#include <string.h>
#include <linux/netfilter_ipv4.h>

#include "utils.h"

int uv_print_error(char* desc, int code) {
  fprintf(stderr, "Error: %s: %s\n", desc, uv_err_name(code));
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

int get_org_dst_addr(int sock_fd, struct sockaddr_storage *orig_dst){
  socklen_t addr_len = sizeof(*orig_dst);

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

void get_addr_str(struct sockaddr_storage *addr, char *addr_str) {
  if(addr->ss_family == AF_INET){
    inet_ntop(AF_INET, &(((struct sockaddr_in*) addr)->sin_addr), addr_str, INET_ADDRSTRLEN);

  } else if (addr->ss_family == AF_INET6){
    inet_ntop(AF_INET6, &(((struct sockaddr_in6*) addr)->sin6_addr), addr_str, INET6_ADDRSTRLEN);

  } else {
    strncpy(addr_str, "(Invalid)", INET6_ADDRSTRLEN);
  }
}

int get_addr_port(struct sockaddr_storage *addr) {
  if(addr->ss_family == AF_INET){
    return ntohs(((struct sockaddr_in*) addr)->sin_port);

  } else if (addr->ss_family == AF_INET6){
    return ntohs(((struct sockaddr_in6*) addr)->sin6_port);

  } else {
    return -1;
  }
}

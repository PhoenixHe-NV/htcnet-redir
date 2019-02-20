cmake_minimum_required(VERSION 3.13)
project(htcnet_redir C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -rdynamic)

add_executable(htcnet_redir
  main.c
  server.c server.h
  utils.c utils.h
  session.c session.h
  buf.c buf.h)

target_link_libraries(htcnet_redir uv)

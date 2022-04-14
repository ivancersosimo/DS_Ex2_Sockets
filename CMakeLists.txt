cmake_minimum_required (VERSION 2.8.11)
project(DS_EX2_SOCKETS)

INCLUDE_DIRECTORIES("include")

#add_executable(client apps/client.c)
#add_executable(server apps/server.c src/lines.c)
add_executable(server apps/server.c src/keys.c)
add_executable(client apps/client.c src/keys.c)

find_package (Threads)
target_link_libraries (server ${CMAKE_THREAD_LIBS_INIT} rt)
target_link_libraries (client ${CMAKE_THREAD_LIBS_INIT} rt)

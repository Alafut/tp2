cmake_minimum_required(VERSION 3.8)
project(tp2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        client/client_thread.c
        client/client_thread.h
        client/main.c
        server/main.c
        server/server_thread.c
        server/server_thread.h)

add_executable(tp2 ${SOURCE_FILES})
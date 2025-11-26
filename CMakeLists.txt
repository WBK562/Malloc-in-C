cmake_minimum_required(VERSION 3.29)
project(for_gh C)

set(CMAKE_C_STANDARD 11)

add_executable(fior_gh main.c
        custom_unistd.h
        heap.c
        memmanager.c)

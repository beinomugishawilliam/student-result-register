
cmake_minimum_required(VERSION 4.4)

project(ASAN)

set(CMAKE_CXX_STANDARD 17)

add_executable(ASAN main.cpp
        Doublylinkedlist.h
        Doublylinkedlist.h)

# Enable AddressSanitizer
target_compile_options(ASAN PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
)

target_link_options(ASAN PRIVATE
        -fsanitize=address
)

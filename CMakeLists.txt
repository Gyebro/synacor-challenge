cmake_minimum_required(VERSION 3.9)
project(synacor_challenge)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,32000000")

add_executable(synacor_challenge
        source/main.cpp
        source/vm.cpp
        source/vm.h
        source/confirmation.cpp
        source/confirmation.h source/debugger.cpp source/debugger.h source/utils.cpp source/utils.h)

add_custom_command(TARGET synacor_challenge POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${PROJECT_SOURCE_DIR}/challenge.bin"
        $<TARGET_FILE_DIR:synacor_challenge>)
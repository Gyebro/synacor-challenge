cmake_minimum_required(VERSION 3.9)
project(synacor_challenge)

set(CMAKE_CXX_STANDARD 11)

add_executable(synacor_challenge main.cpp vm.cpp vm.h confirmation.cpp confirmation.h)

add_custom_command(TARGET synacor_challenge POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${PROJECT_SOURCE_DIR}/challenge.bin"
        $<TARGET_FILE_DIR:synacor_challenge>)
cmake_minimum_required(VERSION 3.26)
project(pdrad C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

#set(tools /opt/arm-gnu-toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu)
#set(CMAKE_C_COMPILER ${tools}/bin/aarch64-linux-gnu-gcc)

include_directories(Inc/)
include_directories(Lib/Inc)

add_executable(pdrad
        Lib/log.c
        Src/test.c
        Src/dh_packets.c
        Src/dh_options.c
        Src/collection.c
        Src/ndp_packets.c
        Src/ndp_options.c
        Src/utils.c
)

add_library(libpdrad
        Lib/log.c
        Src/dh_packets.c
        Src/dh_options.c
        Src/collection.c
        Src/ndp_packets.c
        Src/ndp_options.c
        Inc/utils.h
        Src/utils.c
)

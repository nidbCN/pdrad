cmake_minimum_required(VERSION 3.26)
project(pdrad C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

#set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_LINK_DEPENDS_NO_SHARED TRUE)

set(tools /opt/arm-gnu-toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu)
set(CMAKE_C_COMPILER ${tools}/bin/aarch64-linux-gnu-gcc)

include_directories(Inc/)
include_directories(Lib/Inc)

add_executable(pdrad
        Src/test.c
        Lib/log.c
        Src/utils.c
        Src/collection.c
        Src/dh_packets.c
        Src/dh_options.c
        Src/ndp_packets.c
        Src/ndp_options.c
)

find_package(Threads REQUIRED)
target_link_libraries(pdrad PRIVATE pthread)
target_link_options(pdrad PRIVATE -static)

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

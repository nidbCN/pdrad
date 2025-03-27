cmake_minimum_required(VERSION 3.26)
project(pdrad C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools /opt/arm-gnu-toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu)
set(CMAKE_C_COMPILER ${tools}/bin/aarch64-linux-gnu-gcc)

include_directories(Inc/)
include_directories(Lib/Inc)

add_executable(pdrad
        Lib/log.c
        Src/test.c
        Src/DHCPv6_headers.c
        Src/DHCPv6_options.c
)

add_library(libpdrad
        Lib/log.c
        Src/DHCPv6_headers.c
        Src/DHCPv6_options.c
)

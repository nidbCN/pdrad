#ifndef PDRAD_GLOBAL_H
#define PDRAD_GLOBAL_H
#include <stddef.h>
#include <sys/types.h>
#include <stdlib.h>
#include "log.h"

#define new(T) ((T*)malloc(sizeof(T)))

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    // addr is not const and be fill by each 32bits, this function will modify its content.
#define htobe_inet6(addr) \
    ((uint64_t*)&addr)[0] = htobe64((uint64_t)((uint32_t*)&addr)[0] << 32 | ((uint32_t*)&addr)[1]);\
    ((uint64_t*)&addr)[1] = htobe64((uint64_t)((uint32_t*)&addr)[2] << 32 | ((uint32_t*)&addr)[3]);
#else
    // addr is not const and be fill by each 32bits, this function will modify its content.
    #define htobe_inet6(addr)\
        ptr[0] = htobe32(((uint32_t*)&addr)[0]);\
        ptr[1] = htobe32(((uint32_t*)&addr)[1]);\
        ptr[2] = htobe32(((uint32_t*)&addr)[2]);\
        ptr[3] = htobe32(((uint32_t*)&addr)[3]);
#endif

#define ADDR_All_Nodes_Multicast \
{ \
.__in6_u.__u6_addr32 = {0xff020000, 0x00000000, 0x00000000, 0x00000001} \
}

#define ADDR_All_DHCP_Relay_Agents_and_Servers \
{ \
.__in6_u.__u6_addr32 = {0xff020000, 0x00000000, 0x00000001, 0x00000002} \
}

#endif //PDRAD_GLOBAL_H

#ifndef PDRAD_GLOBAL_H
#define PDRAD_GLOBAL_H

#define new(T) ((T*)malloc(sizeof(T)))

#include <stddef.h>
#include <sys/types.h>
#include <stdlib.h>
#include "log.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    // addr is not const and be fill by each 32bits, this function will modify its content.
#define htobe_inet6(addr) ((uint64_t*)&addr)[0] = htobe64((uint64_t)((uint32_t*)&addr)[0] << 32 | ((uint32_t*)&addr)[1]); ((uint64_t*)&addr)[1] = htobe64((uint64_t)((uint32_t*)&addr)[2] << 32 | ((uint32_t*)&addr)[3]);
#else
    // addr is not const and be fill by each 32bits, this function will modify its content.
    #define htobe_inet6(addr) uint32_t* ptr = (uint32_t*)&addr; ptr[0] = htobe32(ptr[0]); ptr[1] = htobe32(ptr[1]); ptr[2] = htobe32(ptr[2]); ptr[3] = htobe32(ptr[3]);
#endif

#endif //PDRAD_GLOBAL_H

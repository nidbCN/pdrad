#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <netinet/in.h>

#define MAC_ADDRESS_LENGTH 6

/*
 * hwAddress: array length must be longer than 6
 */
int utils_getHardwareAddressByName(uint8_t hwAddress[MAC_ADDRESS_LENGTH]);

int utils_Init(const char *interfaceName);

int utils_getLinkLocalAddress(struct in6_addr *address);

int utils_Dispose();

#endif //UTILS_H

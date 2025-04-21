#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

/*
 * interface: name of interface
 * hwAddress: array length must be longer than 6
 */
int utils_getHardwareAddressByName(const char *interfaceName, uint8_t hwAddress[6]);

int utils_getLinkLocalAddressInit(const char *interfaceName);

int utils_getLinkLocalAddress(struct in6_addr *address);

int utils_getLinkLocalAddressDispose();

#endif //UTILS_H

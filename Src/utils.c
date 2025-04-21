#include "utils.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include "global.h"
#include "log.h"

int utils_getHardwareAddressByName(const char *interfaceName, uint8_t hwAddress[6]) {
#define MAC_ADDRESS_LENGTH 6
    struct ifreq request = {0};
    strcpy(request.ifr_name, interfaceName);

    const int handler = socket(AF_UNIX, SOCK_DGRAM, 0x00);

    if (handler < 0) {
        log_fatal("Can not create socket.(%d: %s)", errno, strerror(errno));
        return errno;
    }

    if (ioctl(handler, SIOCGIFHWADDR, request) < 0) {
        log_error("Cannot get address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return errno;
    }

    memcpy(hwAddress, request.ifr_hwaddr.sa_data, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);

    return 0;
}

int utils_getLinkLocalAddress_Handler = -1;

int utils_getLinkLocalAddressInit(const char *interfaceName) {
    if (utils_getLinkLocalAddress_Handler != -1) {
        return EINVAL;
    }

    utils_getLinkLocalAddress_Handler = socket(AF_INET6, SOCK_DGRAM, 0);

    // create ioctl request to bind to interface
    struct ifreq *request = malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, interfaceName);
    if (setsockopt(utils_getLinkLocalAddress_Handler, SOL_SOCKET, SO_BINDTODEVICE, (char *) request,
                   sizeof(struct ifreq)) < 0) {
        log_error("Bind interface failed: %s.", strerror(errno));
        close(utils_getLinkLocalAddress_Handler);
        free(request);
        return errno;
                   }
    return 0;
}

int utils_getLinkLocalAddress(struct in6_addr *address) {
    struct sockaddr_in6 destAddr = {0};

    struct in6_addr multicastAddr = ADDR_All_Nodes_Multicast;
    htobe_inet6(multicastAddr);

    destAddr.sin6_family = AF_INET6;
    destAddr.sin6_addr = multicastAddr;
    destAddr.sin6_port = htobe16(0);

    // create connection
    if (connect(utils_getLinkLocalAddress_Handler, (struct sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
        log_error("Create connection to link-local failed: %s.", strerror(errno));
        close(utils_getLinkLocalAddress_Handler);
        return errno;
    }

    // get socket connection info
    socklen_t addrLen = sizeof(struct sockaddr_in6);
    if (getsockname(utils_getLinkLocalAddress_Handler, (struct sockaddr *) address, &addrLen) < 0) {
        log_error("Invoke `getsockname` failed: %s.", strerror(errno));
        close(utils_getLinkLocalAddress_Handler);
    }

    return 0;
}

inline int utils_getLinkLocalAddressDispose() {
    return close(utils_getLinkLocalAddress_Handler);
}
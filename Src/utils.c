#include "utils.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include "global.h"
#include "log.h"

int utils_handler = -1;
struct ifreq *utils_request = NULL;

int utils_getHardwareAddressByName(uint8_t hwAddress[MAC_ADDRESS_LENGTH]) {
    utils_request->ifr_ifru.ifru_ivalue = 0;

    if (ioctl(utils_handler, SIOCGIFHWADDR, utils_request) < 0) {
        log_error("Cannot get address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return errno;
    }

    memcpy(hwAddress, utils_request->ifr_hwaddr.sa_data, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);

    return 0;
}

int utils_Init(const char *interfaceName) {
    if (utils_handler != -1 || utils_request != NULL) {
        return EINVAL;
    }

    utils_handler = socket(AF_INET6, SOCK_DGRAM, 0);

    // create ioctl request to bind to interface
    utils_request = malloc(sizeof(struct ifreq));
    strcpy(utils_request->ifr_name, interfaceName);
    if (setsockopt(utils_handler, SOL_SOCKET, SO_BINDTODEVICE, utils_request,
                   sizeof(struct ifreq)) < 0) {
        log_error("Bind interface failed: %s.", strerror(errno));
        close(utils_handler);
        free(utils_request);
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
    if (connect(utils_handler, (struct sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
        log_error("Create connection to link-local failed: %s.", strerror(errno));
        close(utils_handler);
        return errno;
    }

    // get socket connection info
    socklen_t addrLen = sizeof(struct sockaddr_in6);
    if (getsockname(utils_handler, (struct sockaddr *) address, &addrLen) < 0) {
        log_error("Invoke `getsockname` failed: %s.", strerror(errno));
        close(utils_handler);
    }

    return 0;
}

inline int utils_Dispose() {
    log_warn("Close ioctl handler :%d.", utils_handler);
    return close(utils_handler);
}
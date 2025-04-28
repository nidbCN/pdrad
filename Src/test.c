#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "dh_packets.h"
#include "global.h"
#include "dh_options.h"
#include "log.h"
#include "ndp_packets.h"
#include "utils.h"
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_PORT 547
#define CLIENT_PORT 546

bool Thread_sendRouterAdv_cancel = false;
bool Thread_requestDhcpPrefix_cancel = false;
bool Thread_listenRouterSolicit_cancel = false;

bool _prefixLock = false;
dh_opt_IA_Prefix _prefix = {0x00};

void handle(int _) {
    Thread_sendRouterAdv_cancel = true;
    Thread_listenRouterSolicit_cancel = true;
    Thread_requestDhcpPrefix_cancel = true;

    utils_Dispose();
}

void print_memory_hex(const void *mem, const size_t size) {
    const unsigned char *byte = mem;
    for (size_t i = 0; i < size; i++) {
        printf("%02X", byte[i]);
        if (i < size - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

int requestAndReceiveDhcp(int handler, void *buffer, size_t bufferLength,
                          struct in6_addr clientAddr,
                          dh_opt_IA_Prefix *prefixResultPtr);

struct in6_addr ReceiveSrcAddrFromRouterSolicit(int handler);

int sendRouterAdv(int handler, uint8_t macAddr[6], struct in6_addr linkLocalAddr, struct in6_addr dstAddr,
                  struct in6_addr prefix, uint prefixLength, uint validTime, uint preferTime);

/*
 * RA Thread arguments
 *
 */
struct Thread_sendRouterAdv_Arg {
    const char *interfaceName;
    int intervalSec;
};

void *Thread_sendRouterAdv(struct Thread_sendRouterAdv_Arg *arg) {
    log_info("RA thread start, bind interface {%s}, intervalSec {%d}", arg->interfaceName, arg->intervalSec);

    // ndp socket, IPv6 with next header ICMPv6 RAW SOCK
    const int ndpHandler = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (ndpHandler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct ifreq *bindToLanDevReq = malloc(sizeof(struct ifreq));
    strcpy(bindToLanDevReq->ifr_name, arg->interfaceName);

    if (setsockopt(ndpHandler, SOL_SOCKET, SO_BINDTODEVICE, (char *) bindToLanDevReq, sizeof(struct ifreq)) < 0) {
        log_error("Bind to interface %s failed", bindToLanDevReq->ifr_name);
        log_error(strerror(errno));
        close(ndpHandler);
        free(bindToLanDevReq);
        return NULL;
    }

    struct in6_addr dstAddr = ADDR_All_Nodes_Multicast;
    htobe_inet6(dstAddr);

    while (!Thread_sendRouterAdv_cancel) {
        struct in6_addr srcAddr = {0};
        utils_getLinkLocalAddress(&srcAddr);

        uint8_t lanIfHwAddr[6] = {0};
        utils_getHardwareAddressByName(lanIfHwAddr);

        while (_prefixLock);
        _prefixLock = true;

        struct in6_addr prefix = {0x00};
        memcpy(&prefix, _prefix.Prefix, sizeof(struct in6_addr));

        log_info("Send a RA at interval %d.", arg->intervalSec);
        sendRouterAdv(ndpHandler, lanIfHwAddr, srcAddr,
                      dstAddr,
                      prefix,
                      _prefix.PrefixLength,
                      _prefix.ValidLifetime,
                      _prefix.PreferredLifetime
        );

        _prefixLock = false;
        sleep(arg->intervalSec);
    }

    close(ndpHandler);

    return NULL;
}

void *Thread_requestDhcpPrefix(const char *wanIfName) {
    // init a udp(DHCPv6) socket
    int handler = socket(AF_INET6, SOCK_DGRAM, 0);
    if (handler < 0) {
        log_error("DHCP Socket creation failed: %s.", strerror(errno));

        exit(EXIT_FAILURE);
    }

    // ioctl req
    struct ifreq *request = malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, wanIfName);

    // bind to interface
    if (setsockopt(handler, SOL_SOCKET, SO_BINDTODEVICE, request, sizeof(struct ifreq)) < 0) {
        log_error("Bind interface %s failed: %s.", request->ifr_name, strerror(errno));

        close(handler);
        free(request);
        return NULL;
    }

//    // bind to multicast
//    if (setsockopt(handler, IPPROTO_UDP, IPV6_MULTICAST_IF, request, sizeof(struct ifreq)) < 0) {
//        log_error("Bind to multicast %s failed: %s.", request->ifr_name, strerror(errno));
//
//        close(handler);
//        free(request);
//        return NULL;
//    }

    struct sockaddr_in6 client_addr = {0x00};
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(CLIENT_PORT);
    client_addr.sin6_addr = in6addr_any;
//
//    // bind address
//    if (bind(handler, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
//        log_error("Bind address  failed");
//        log_error(strerror(errno));
//        close(handler);
//        return NULL;
//    }

    // query mtu
    int mtu = 1500;
    if (ioctl(handler, SIOCGIFMTU, request) >= 0) {
        mtu = request->ifr_ifru.ifru_mtu;
    } else {
        log_error("ioctl error, can't get mtu: %s", strerror(errno));
    }

    free(request);

    void *buffer = malloc(mtu);
    memset(buffer, 0x00, mtu);

    while (!Thread_requestDhcpPrefix_cancel) {
        while (_prefixLock);
        _prefixLock = true;

        requestAndReceiveDhcp(handler, buffer, mtu, client_addr.sin6_addr, &_prefix);
        int lifeTime = be32toh(_prefix.PreferredLifetime);
        char *prefixStr = alloca(sizeof(char) * 40);

        inet_ntop(AF_INET6, _prefix.Prefix, prefixStr, INET6_ADDRSTRLEN);
        log_info("Parse success, prefix=`%s/%d`, preferred lifetime=%d, lifetime=%d.",
                 prefixStr,
                 _prefix.PrefixLength,
                 lifeTime,
                 be32toh(_prefix.ValidLifetime));

        _prefixLock = false;

        sleep(lifeTime);
    }

    close(handler);
    free(buffer);
    return NULL;
}

void Thread_listenRouterSolicit(const char *lanIfName) {
    log_info("RS thread start, bind interface {%s}", lanIfName);

    // ndp socket, IPv6 with next header ICMPv6 RAW SOCK
    const int ndpMulticastHandler = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (ndpMulticastHandler < 0) {
        log_error("Socket creation failed: %s.", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ndp socket, IPv6 with next header ICMPv6 RAW SOCK
    const int ndpHandler = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (ndpHandler < 0) {
        log_error("Socket creation failed: %s.", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct ifreq bindToLanDevReq = {0x00};
    strcpy(bindToLanDevReq.ifr_ifrn.ifrn_name, lanIfName);

    // bind interface
    if (setsockopt(ndpMulticastHandler, SOL_SOCKET, SO_BINDTODEVICE,
                   &bindToLanDevReq, sizeof(struct ifreq)) < 0) {
        log_error("Bind multicast socket to interface %s failed: %s.", bindToLanDevReq.ifr_ifrn.ifrn_name,
                  strerror(errno));
        close(ndpMulticastHandler);
        return;
    }

    // bind interface
    if (setsockopt(ndpHandler, SOL_SOCKET, SO_BINDTODEVICE,
                   (char *) &bindToLanDevReq, sizeof(struct ifreq)) < 0) {
        log_error("Bind socket to interface %s failed: %s.", bindToLanDevReq.ifr_ifrn.ifrn_name, strerror(errno));
        close(ndpHandler);
        return;
    }

    struct ipv6_mreq multicastReq = {0x00};
    struct in6_addr multicastAddr = ADDR_All_Routers_Multicast;
    htobe_inet6(multicastAddr);
    multicastReq.ipv6mr_multiaddr = multicastAddr;
    multicastReq.ipv6mr_interface = if_nametoindex(lanIfName);

    // join multicast
    if (setsockopt(ndpMulticastHandler, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                   &multicastReq, sizeof(struct ipv6_mreq)) < 0) {
        log_error("Join multicast failed: %s.", strerror(errno));

        close(ndpMulticastHandler);
        return;
    }

    struct sockaddr_in6 bindTo = {0x00};
    bindTo.sin6_addr = in6addr_any;
    bindTo.sin6_family = AF_INET6;

    if (bind(ndpMulticastHandler, (const struct sockaddr *) &bindTo, sizeof(bindTo)) < 0) {
        log_error("Bind to any failed: %s.", strerror(errno));
    }

    while (!Thread_listenRouterSolicit_cancel) {
        struct in6_addr linkLocalAddr = {0};
        utils_getLinkLocalAddress(&linkLocalAddr);

        struct in6_addr clientAddr = ReceiveSrcAddrFromRouterSolicit(ndpMulticastHandler);

        log_info("Success parse client address from RS, reply a RA.");

        uint8_t lanIfHwAddr[6] = {0};
        utils_getHardwareAddressByName(lanIfHwAddr);

        _prefixLock = true;

        struct in6_addr prefix = {0x00};
        memcpy(&prefix, _prefix.Prefix, sizeof(struct in6_addr));

        log_info("Send a RA.");

        sendRouterAdv(ndpMulticastHandler, lanIfHwAddr, linkLocalAddr, clientAddr,
                      prefix,
                      _prefix.PrefixLength,
                      _prefix.ValidLifetime,
                      _prefix.PreferredLifetime
        );

        _prefixLock = false;
    }

    close(ndpMulticastHandler);
    close(ndpHandler);
}

int main(int argc, char *argv[]) {
    bool useDhcp = false;
    const char *wanIfName = NULL;

    if (argc <= 1) {
        log_error("Invalid argument.\n"
                  "Usage: \n"
                  "\t%s d <lan interface name> <wan interface name>\n"
                  "\t%s s <lan interface name> <prefix> <prefix length>",
                  argv[0], argv[0]
        );

        return -1;
    }

    if (argc >= 3 || (*argv[1] != 'd' && *argv[2] != 's')) {
        if (*argv[1] == 'd') {
            useDhcp = true;
            wanIfName = argv[3];
        } else if (*argv[1] == 's') {
            struct in6_addr staticPrefix = {0x00};
            inet_pton(AF_INET6, argv[3], &staticPrefix);

            memcpy(_prefix.Prefix, &staticPrefix, sizeof(staticPrefix));

            _prefix.PreferredLifetime = 1296000;
            _prefix.ValidLifetime = 2592000;
            _prefix.PrefixLength = atoi(argv[4]);
        } else {
            log_error("Usage: \n"
                      "\t%s d <lan interface name> <wan interface name>"
                      "\t%s s <lan interface name> <prefix> <prefix length>",
                      argv[0], argv[0]
            );

            return -1;
        }
    } else {
        log_error("Usage: \n"
                  "\t%s d <lan interface name> <wan interface name>"
                  "\t%s s <lan interface name> <prefix> <prefix length>",
                  argv[0], argv[0]
        );

        return -1;
    }

    const char *lanIfName = argv[2];

    utils_Init(lanIfName);

    // REG SIG
    log_info("Register signal handler for SIGINT");
    if (SIG_ERR == signal(SIGINT, handle)) {
        log_error("Register signal handler for SIGINT failed");
        return -1;
    }
    log_info("Register signal handler for SIGTERM");
    if (SIG_ERR == signal(SIGTERM, handle)) {
        log_error("Register signal handler for SIGTERM failed");
        return -1;
    }

    if (useDhcp) {
        pthread_t thread_dhcp;
        pthread_create(&thread_dhcp, NULL, (void *(*)(void *)) Thread_requestDhcpPrefix, (void *) wanIfName);

        // wait for a result
        while (_prefix.PrefixLength == 0);
    }

    pthread_t thread_ra;
    pthread_t thread_rs;

    struct Thread_sendRouterAdv_Arg sendRouterAdvArg = {
            .interfaceName = lanIfName,
            .intervalSec = 200,
    };

    pthread_create(&thread_ra, NULL, (void *(*)(void *)) Thread_sendRouterAdv, (void *) &sendRouterAdvArg);
    pthread_create(&thread_rs, NULL, (void *(*)(void *)) Thread_listenRouterSolicit, (void *) lanIfName);

    while (true) {
        sleep(1);
    }
}

int sendRouterAdv(const int handler, uint8_t macAddr[6], struct in6_addr linkLocalAddr, struct in6_addr dstAddr,
                  struct in6_addr prefix, uint prefixLength, uint validTime, uint preferTime) {
    log_info("Start to send RA.");
#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    ndp_optPayload *prefixInfo = ndp_createOptionPrefixInformation(
            prefixLength,
            (ndp_opt_PrefixInformation_flag_L | ndp_opt_PrefixInformation_flag_A) & ndp_opt_PrefixInformation_flag_R,
            validTime,
            preferTime,
            prefix);
#pragma clang diagnostic pop

    ndp_optPayload *routeInfo = ndp_createOptionRouteInformation(56, 0x00, validTime, prefix);
    ndp_optPayload *mtu = ndp_createOptionMtu(1500);
    ndp_optPayload *linkAddress = ndp_createOptionSourceLinkLayerAddress(macAddr);

    struct nd_router_advert *raPacket = NULL;

    size_t pktSize = ndp_createRAPacket(
            &raPacket,
            linkLocalAddr,
            dstAddr,
            64,
            1800,
            0x00,
            0x00,
            4,
            linkAddress, prefixInfo, routeInfo, mtu);

    struct sockaddr_in6 dstAddrSock = {0x00};
    dstAddrSock.sin6_family = AF_INET6;
    dstAddrSock.sin6_port = 0x00;
    dstAddrSock.sin6_addr = dstAddr;

    char buffer[40] = {0};
    inet_ntop(AF_INET6, &dstAddr, buffer, 40);

    log_info("Send RA to %s.", buffer);
    if (sendto(handler, raPacket, pktSize,
               0, (const struct sockaddr *) &dstAddrSock,
               sizeof(struct sockaddr_in6)) < 0) {
        log_error("Send RA failed: %s.", strerror(errno));
    }

    free(prefixInfo);
    free(mtu);
    free(linkAddress);
    free(raPacket);

    return 0;
}

struct in6_addr ReceiveSrcAddrFromRouterSolicit(const int handler) {
    int mtu = 1500;
    void *recBuf = alloca(mtu);

    struct sockaddr_in6 addr = {0x00};
    int addrLen = sizeof(struct sockaddr_in6);

    while (true) {
        ssize_t recLen = recvfrom(handler, recBuf, mtu, MSG_WAITFORONE, (struct sockaddr *) &addr,
                                  (socklen_t *) &addrLen);

        log_info("RS socket received packet with %ld bytes.", recLen);

        // 解析 ICMPv6 头
        struct nd_router_solicit *rs = (struct nd_router_solicit *) (recBuf);
        if (rs->nd_rs_hdr.icmp6_type != 133) {
            log_info("RS type not 133, drop.");
            continue;
        }
        if (rs->nd_rs_hdr.icmp6_code != 0) {
            log_info("RS code not 0, drop.");
            continue;
        }
        // skip check sum

        log_info("Received a RS.");

        return addr.sin6_addr;
    }
}

int requestAndReceiveDhcp(const int handler, void *buffer, size_t bufferLength,
                          struct in6_addr clientAddr,
                          dh_opt_IA_Prefix *prefixResultPtr) {
    // CLIENT ID
    //      opt type enid id
    uint8_t id[] = {0x0d, 0x00, 0x07, 0x21};
    dh_optPayload *clientId = dh_createOption_ClientIdentifier_En(4107, id, 4);

    if (clientId == NULL) {
        log_error("DHCPv6 CLIENT ID payload init failed.");
        exit(EXIT_FAILURE);
    }
    log_info("DHCPv6 CLIENT ID payload create success.\n");

    // ES TIME
    //    opt time
    dh_optPayload *esTime = dh_createOption_ElapsedTime(0);
    if (esTime == NULL) {
        log_error("DHCPv6 ELAPSED TIME payload init failed.");
        exit(EXIT_FAILURE);
    }

    log_info("DHCPv6 ELAPSED TIME payload create success.\n");

    // IA PD
    dh_optPayload *IA_Prefix = dh_createOption_IA_PD(0x0d0007021, NULL, 0);

    if (IA_Prefix == NULL) {
        log_error("DHCPv6 PD payload init failed.");
        exit(EXIT_FAILURE);
    }
    log_info("DHCPv6 PD payload create success.\n");

    // RAPID COMMIT
    //   opt rapid
    dh_optPayload *rapid = dh_createOption_RapidCommit();
    if (rapid == NULL) {
        log_error("DHCPv6 RAPID COMMIT payload init failed.");
        exit(EXIT_FAILURE);
    }
    log_info("DHCPv6 RAPID COMMIT payload create success.\n");

    const dh_pkt *pktPtr = NULL;
    size_t pktLength = dh_CreateCustomizedSolicitPacket(&pktPtr, clientId, esTime, IA_Prefix, 1, rapid);

    // has been copy to packet
    free(clientId);
    free(esTime);
    free(IA_Prefix);
    free(rapid);

    log_info("Ready to send");
    print_memory_hex(pktPtr, pktLength);

    struct in6_addr serverIp = ADDR_All_DHCP_Relay_Agents_and_Servers;
    htobe_inet6(serverIp);

    struct sockaddr_in6 sockServerAddr = {0x00};
    sockServerAddr.sin6_family = AF_INET6;
    sockServerAddr.sin6_port = htons(SERVER_PORT);
    sockServerAddr.sin6_addr = serverIp;

    if (sendto(handler, pktPtr, pktLength, 0,
               (struct sockaddr *) &sockServerAddr, sizeof(sockServerAddr)) < 0) {
        log_error("Send failed.");
        log_error(strerror(errno));
        close(handler);
        return 1;
    }

    log_info("Send success.");

    socklen_t addrLen = sizeof(struct sockaddr);
    ssize_t recLen = recvfrom(handler, buffer, bufferLength, 0,
                              (struct sockaddr *) &clientAddr, &addrLen);

    if (recLen < 0) {
        log_error("`recvfrom` call failed: %s", strerror(errno));
    }

    log_info("received %ld bytes.", recLen);

    const dh_pkt *recBuf = buffer;
    print_memory_hex(recBuf, recLen);

    log_info("msg-type: %d, trans-id: 0x%x%x%x.", recBuf->MsgType,
             recBuf->TransactionId.TransactionId_0,
             recBuf->TransactionId.TransactionId_1,
             recBuf->TransactionId.TransactionId_2);

    dh_parsedOptions parsed = dh_parseOptions(recBuf, recLen);
    if (!parsed.success) {
        log_error("parse failed.");
    }

    dh_optPayload *pd = parsed.IA_PDList->value;
    dh_opt_IA_PD *pdData = (dh_opt_IA_PD *) pd->OptionData;
    dh_optPayload *pdOption = (dh_optPayload *) pdData->Options;
    dh_opt_IA_Prefix *prefix = (dh_opt_IA_Prefix *) pdOption->OptionData;

    memcpy(prefixResultPtr, prefix, sizeof(dh_opt_IA_Prefix));
    close(handler);

    return 0;
}

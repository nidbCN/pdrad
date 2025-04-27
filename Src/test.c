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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <bits/signum-generic.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_PORT 547
#define CLIENT_PORT 546

bool Thread_sendRouterAdv_cancel = false;
bool Thread_requestDhcpPrefix_cancel = false;
bool Thread_listenRouterSolicit_cancel = false;

dh_opt_IA_Prefix *_prefix = NULL;

void handle(int sig) {
    Thread_sendRouterAdv_cancel = true;
    Thread_listenRouterSolicit_cancel = true;
    Thread_requestDhcpPrefix_cancel = true;

    utils_getLinkLocalAddressDispose();
}

void print_binary(const uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

void print_memory_little_endian(void *ptr, const size_t size) {
    const uint8_t *bytes = (uint8_t *) ptr;
    printf("Little Endian: ");
    for (size_t i = 0; i < size; i++) {
        print_binary(bytes[i]);
        printf(" ");
    }
    printf("\n");
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

int requestAndReceiveDhcp();

struct in6_addr ReceiveSrcAddrFromRouterSolicit(int handler, struct in6_addr linkLocalAddr);

int sendRouterAdv(int handler, uint8_t macAddr[6], struct in6_addr linkLocalAddr, struct in6_addr dstAddr);

void Thread_sendRouterAdv(const char *lanIfName, uint intervalSec) {
    log_info("RA thread start, bind interface {%s}, intervalSec {%d}", lanIfName, intervalSec);

    // ndp socket, IPv6 with next header ICMPv6 RAW SOCK
    const int ndpHandler = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (ndpHandler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct ifreq *bindToLanDevReq = malloc(sizeof(struct ifreq));
    strcpy(bindToLanDevReq->ifr_name, lanIfName);

    if (setsockopt(ndpHandler, SOL_SOCKET, SO_BINDTODEVICE, (char *) bindToLanDevReq, sizeof(struct ifreq)) < 0) {
        log_error("Bind to interface %s failed", bindToLanDevReq->ifr_name);
        log_error(strerror(errno));
        close(ndpHandler);
        free(bindToLanDevReq);
        return;
    }

    struct in6_addr dstAddr = ADDR_All_Nodes_Multicast;
    htobe_inet6(dstAddr);

    while (!Thread_sendRouterAdv_cancel) {
        struct in6_addr srcAddr = {0};
        utils_getLinkLocalAddress(&srcAddr);

        uint8_t lanIfHwAddr[6] = {0};
        utils_getHardwareAddressByName(lanIfName, lanIfHwAddr);
        sendRouterAdv(ndpHandler, lanIfHwAddr, srcAddr, dstAddr);
        sleep(intervalSec);
    }

    close(ndpHandler);
}

void Thread_requestDhcpPrefix(const char *wanIfName) {
    // init a udp socket
    int handler = socket(AF_INET6, SOCK_DGRAM, 0);
    if (handler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ioctl req
    struct ifreq *request = malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, wanIfName);
    if (setsockopt(handler, IPPROTO_UDP, IPV6_MULTICAST_IF, (char *) request, sizeof(struct ifreq)) < 0) {
        log_error("Bind interface failed");
        log_error(strerror(errno));
        close(handler);
        free(request);
        return;
    }

    struct sockaddr_in6 client_addr = {0x00};
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(CLIENT_PORT);
    client_addr.sin6_addr = in6addr_any;

    // bind interface
    if (bind(handler, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        log_error("Bind failed");
        log_error(strerror(errno));
        close(handler);
        return;
    }

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


    free(buffer);
}

void Thread_listenRouterSolicit(const char *lanIfName) {
    log_info("RS thread start, bind interface {%s}", lanIfName);

    // ndp socket, IPv6 with next header ICMPv6 RAW SOCK
    const int ndpHandler = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (ndpHandler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct ifreq *bindToLanDevReq = malloc(sizeof(struct ifreq));
    strcpy(bindToLanDevReq->ifr_name, lanIfName);

    if (setsockopt(ndpHandler, SOL_SOCKET, SO_BINDTODEVICE, (char *) bindToLanDevReq, sizeof(struct ifreq)) < 0) {
        log_error("Bind to interface %s failed", bindToLanDevReq->ifr_name);
        log_error(strerror(errno));
        close(ndpHandler);
        free(bindToLanDevReq);
        return;
    }

    while (!Thread_listenRouterSolicit_cancel) {
        struct in6_addr linkLocalAddr = {0};
        utils_getLinkLocalAddress(&linkLocalAddr);
        struct in6_addr clientAddr = ReceiveSrcAddrFromRouterSolicit(ndpHandler, linkLocalAddr);

        uint8_t lanIfHwAddr[6] = {0};
        utils_getHardwareAddressByName(lanIfName, lanIfHwAddr);
        sendRouterAdv(ndpHandler, lanIfHwAddr, linkLocalAddr, clientAddr);
    }

    close(ndpHandler);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        log_error("Usage: %s <lan interface name> <wan interface name>", argv[0]);
    }

    utils_getLinkLocalAddressInit(argv[1]);

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

    pthread_create(NULL, NULL, Thread_sendRouterAdv, (void *) lanIfName, 1000);

    utils_getLinkLocalAddressDispose();
    return 0;
}

int sendRouterAdv(const int handler, uint8_t macAddr[6], struct in6_addr linkLocalAddr, struct in6_addr dstAddr) {
    struct in6_addr testPrefix = {
            .__in6_u.__u6_addr32 = {0xfd320026, 0x0d000721, 0x00000000, 0x00000000}
    };
    htobe_inet6(testPrefix);

    ndp_optPayload *prefixInfo = ndp_createOptionPrefixInformation(
            64,
            (ndp_opt_PrefixInformation_flag_L | !ndp_opt_PrefixInformation_flag_A) & ndp_opt_PrefixInformation_flag_R,
            60,
            30,
            testPrefix);

    ndp_optPayload *mtu = ndp_createOptionMtu(1500);
    ndp_optPayload *linkAddress = ndp_createOptionSourceLinkLayerAddress(macAddr);

    struct nd_router_advert *raPacket = NULL;

    size_t pktSize = ndp_createRAPacket(
            &raPacket,
            linkLocalAddr,
            dstAddr,
            64,
            60,
            0x00,
            0x00,
            3,
            prefixInfo, mtu, linkAddress);

    sendto(handler, raPacket, pktSize,
           0, (struct sockaddr *) &dstAddr,
           sizeof(dstAddr));

    free(prefixInfo);
    free(mtu);
    free(linkAddress);
    free(raPacket);

    return 0;
}

struct in6_addr ReceiveSrcAddrFromRouterSolicit(const int handler, struct in6_addr linkLocalAddr) {
    int mtu = 1500;
    void *recBuf = alloca(mtu);

    size_t addrLen = sizeof(linkLocalAddr);

    while (true) {
        ssize_t recLen = recvfrom(handler, recBuf, mtu, MSG_WAITALL, (struct sockaddr *) &linkLocalAddr,
                                  (socklen_t *) &addrLen);

        if (recLen < 14 + sizeof(struct ip6_hdr)) {
            continue; // 忽略无效包
        }
        struct ip6_hdr *ip6 = (struct ip6_hdr *) (recBuf + 14);

        struct in6_addr multCast = ADDR_All_Nodes_Multicast;
        // 检查目标地址
        if (memcmp(&ip6->ip6_dst, &multCast, sizeof(struct in6_addr)) < sizeof(struct in6_addr)) {
            continue;
        }

        // 检查协议是否为 ICMPv6
        if (ip6->ip6_nxt != IPPROTO_ICMPV6) {
            continue;
        }

        // 解析 ICMPv6 头
        struct nd_router_solicit *rs = (struct nd_router_solicit *) (ip6 + 1);
        if (rs->nd_rs_hdr.icmp6_type != 133)
            continue;
        if (rs->nd_rs_hdr.icmp6_code != 0)
            continue;

        // skip check sum

        return ip6->ip6_src;
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

    char *prefixStr = alloca(sizeof(char) * 40);

    inet_ntop(AF_INET6, prefix->Prefix, prefixStr, INET6_ADDRSTRLEN);
    log_info("Parse success, prefix=`%s/%d`, preferred lifetime=%d, lifetime=%d.",
             prefixStr,
             prefix->PrefixLength,
             be32toh(prefix->PreferredLifetime),
             be32toh(prefix->ValidLifetime));

    close(handler);

    return 0;
}

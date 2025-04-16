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
#include "dh_options.h"
#include "log.h"

#define SERVER_ADDR "ff02::1:2"

#define SERVER_PORT 547
#define CLIENT_PORT 546

void print_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

void print_memory_little_endian(void *ptr, size_t size) {
    uint8_t *bytes = (uint8_t *) ptr;
    printf("Little Endian: ");
    for (size_t i = 0; i < size; i++) {
        print_binary(bytes[i]);
        printf(" ");
    }
    printf("\n");
}

void print_memory_hex(const void *mem, size_t size) {
    const unsigned char *byte = (const unsigned char *) mem;
    for (size_t i = 0; i < size; i++) {
        printf("%02X", byte[i]);
        if (i < size - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

int sendAndReceivedDhcpPd();

int main() {
    sendAndReceivedDhcpPd();
    return 0;
}

int sendRA() {
    char *ifName = "eth1";

    // init a udp socket
    int handler = socket(AF_INET6, SOCK_RAW, 0);
    if (handler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ioctl req
    struct ifreq *request = (struct ifreq *) malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, ifName);
    if (setsockopt(handler, SOL_SOCKET, SO_BINDTODEVICE, (char *) request, sizeof(struct ifreq)) < 0) {
        log_error("Bind interface failed");
        log_error(strerror(errno));
        close(handler);
        free(request);
        return 1;
    }

    if (ioctl(handler, SIOCGIFADDR, request) < 0) {
        log_error("ioctl error, can't get addr: %s", strerror(errno));
    }

    if (request != NULL) {
        //  struct sockaddr mtu = request->ifr_ifru.ifru_addr.sa_family;


    }
}

int sendAndReceivedDhcpPd() {
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

    // init a udp socket
    int handler = socket(AF_INET6, SOCK_DGRAM, 0);
    if (handler < 0) {
        log_error("Socket creation failed.");
        log_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ioctl req
    struct ifreq *request = (struct ifreq *) malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, "ppp0");
    if (setsockopt(handler, SOL_SOCKET, SO_BINDTODEVICE, (char *) request, sizeof(struct ifreq)) < 0) {
        log_error("Bind interface failed");
        log_error(strerror(errno));
        close(handler);
        free(request);
        return 1;
    }

    // request in use

    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(CLIENT_PORT);
    client_addr.sin6_addr = in6addr_any;

    if (bind(handler, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        log_error("Bind failed");
        log_error(strerror(errno));
        close(handler);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);
    inet_pton(AF_INET6, SERVER_ADDR, &server_addr.sin6_addr);

    if (sendto(handler, pktPtr, pktLength, 0,
               (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        log_error("Send failed.");
        log_error(strerror(errno));
        close(handler);
        return 1;
    }

    log_info("Send success.");

    int mtu = 1500;

    if (ioctl(handler, SIOCGIFMTU, request) < 0) {
        log_error("ioctl error, can't get mtu: %s", strerror(errno));
    }

    if (request != NULL) {
        mtu = request->ifr_ifru.ifru_mtu;
    }

    free(request);

    dh_pkt *recBuf = (dh_pkt *) malloc(mtu);

    socklen_t addrLen = sizeof(client_addr);
    ssize_t recLen = recvfrom(handler, recBuf, mtu, 0,
                              (struct sockaddr *) &client_addr, &addrLen);

    if (recLen < 0) {
        log_error("`recvfrom` call failed: %s", strerror(errno));
    }

    log_info("received %ld bytes.", recLen);
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

    char *prefixStr = alloca(sizeof(char) * 40);

    inet_ntop(AF_INET6, prefix->Prefix, prefixStr, INET6_ADDRSTRLEN);
    log_info("Parse success, prefix=`%s/%d`, preferred lifetime=%d, lifetime=%d.",
             prefixStr,
             prefix->PrefixLength,
             be32toh(prefix->PreferredLifetime),
             be32toh(prefix->ValidLifetime)
    );

    free(recBuf);
    close(handler);

    return 0;
}
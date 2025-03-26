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
#include "DHCPv6_header.h"
#include "DHCPv6_options.h"

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

int main() {
    void *p = malloc(sizeof(void *));
    srandom((unsigned int) p);
    free(p);

    int pktLen = 4   // hdr
                 + (4 + 2 + 4 + 4)     // CLIENT ID
                 + (4 + sizeof(DHCPv6_opt_ElapsedTime))  // E TIME
                 + (4)   // RAPID COMMIT
                 + (4 + sizeof(DHCPv6_opt_IA_PD));  // IA PD
    DHCPv6_pkt *pkt = (DHCPv6_pkt *) malloc(pktLen);
    pkt->MsgType = SOLICIT;
    pkt->TransactionId[0] = random() % 256;
    pkt->TransactionId[1] = random() % 256;
    pkt->TransactionId[2] = random() % 256;
    void *option = pkt->Options;

    // CLIENT ID
    //      opt type enid id
    int len = 4 + 2 + 4 + 4;
    DHCPv6_optPayload *buffer;

    uint8_t id[] = {0x0d, 0x00, 0x07, 0x21};
    buffer = (DHCPv6_optPayload *) malloc(len);
    if (!createOption_ClientIdentifier_En(buffer, 4107, id, 4)) {
        perror("DHCPv6 CLIENT ID payload init failed.");
        exit(EXIT_FAILURE);
    }
    printf("DHCPv6 CLIENT ID payload create success.\n");
    print_memory_hex(buffer, len);
    memcpy(option, buffer, len);
    option += len;
    free(buffer);

    // ES TIME
    //    opt time
    len = 4 + sizeof(DHCPv6_opt_ElapsedTime);
    buffer = (DHCPv6_optPayload *) malloc(len);
    if (!createOption_ElapsedTime(buffer, 0)) {
        perror("DHCPv6 ELAPSED TIME payload init failed.");
        exit(EXIT_FAILURE);
    }
    printf("DHCPv6 ELAPSED TIME payload create success.\n");
    print_memory_hex(buffer, len);
    memcpy(option, buffer, len);
    option += len;
    free(buffer);

    // RAPID COMMIT
    //   opt rapid
    len = 4 + 0;
    buffer = (DHCPv6_optPayload *) malloc(len);
    if (!createOption_RapidCommit(buffer)) {
        perror("DHCPv6 RAPID COMMIT payload init failed.");
        exit(EXIT_FAILURE);
    }
    printf("DHCPv6 RAPID COMMIT payload create success.\n");
    print_memory_hex(buffer, len);
    memcpy(option, buffer, len);
    option += len;
    free(buffer);

    // IA PD
    len = 4 + sizeof(DHCPv6_opt_IA_PD);
    buffer = (DHCPv6_optPayload *) malloc(len);
    if (!createOption_IA_PD(buffer, 0x0d000721, 8192, 8192)) {
        perror("DHCPv6 PD payload init failed.");
        exit(EXIT_FAILURE);
    }
    printf("DHCPv6 PD payload create success.\n");
    print_memory_hex(buffer, len);
    memcpy(option, buffer, len);
    option += len;
    free(buffer);

    printf("Ready to send");
    print_memory_hex(pkt, pktLen);

    // init a udp socket
    int handler = socket(AF_INET6, SOCK_DGRAM, 0);
    if (handler < 0) {
        perror("Socket creation failed.");
        printf(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ioctl req
    struct ifreq *request = (struct ifreq *) malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, "ppp0");
    setsockopt(handler, SOL_SOCKET, SO_BINDTODEVICE, (char *) request, sizeof(struct ifreq));

    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(CLIENT_PORT);
    client_addr.sin6_addr = in6addr_any;

    if (bind(handler, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        perror("Bind failed");
        printf(strerror(errno));
        close(handler);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);
    inet_pton(AF_INET6, SERVER_ADDR, &server_addr.sin6_addr);

    if (sendto(handler, pkt, pktLen, 0,
               (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed.");
        printf(strerror(errno));
        close(handler);
        return 1;
    }

    printf("Send success.");
    printf(strerror(errno));

//    while (true) {
    int mtu = 1500;

    if (ioctl(handler, SIOCGIFMTU, request) < 0) {
        perror("ioctl");
    }

    mtu = request->ifr_ifru.ifru_mtu;
    DHCPv6_pkt *recBuf = (DHCPv6_pkt *) malloc(mtu);

    socklen_t addrLen = sizeof(client_addr);
    ssize_t recLen = recvfrom(handler, recBuf, mtu, 0,
                              (struct sockaddr *) &client_addr, &addrLen);

    if (recLen < 0) {
        perror("recvfrom 失败");
//            continue;
    }

    printf("rec %ld bytes\n", recLen);
    print_memory_hex(recBuf, recLen);

    printf("msg-type: %d, trans-id: %d,%d,%d\n", pkt->MsgType, pkt->TransactionId[0], pkt->TransactionId[1],
           pkt->TransactionId[1]);

//        int hasRead = 4;
//        while (hasRead < recLen) {
//            int ret = readOptionFromCSMessage(buffer, pkt, recLen, 0, 0);
//        }
//}

    close(handler);
    return 0;
}


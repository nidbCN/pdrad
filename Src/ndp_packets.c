#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <netinet/icmp6.h>
#include "ndp_packets.h"
#include "ndp_options.h"

size_t ndp_createRAPacket(
        struct nd_router_advert **pktBufPtr,
        const struct in6_addr sourceAddr,
        const struct in6_addr destAddr,
        const uint8_t curHopLimit,
        const uint16_t routerLifeTime,
        const uint32_t reachableTime,
        const uint32_t reTransTimer,
        uint optionsNum,
        ...) {

    size_t length = sizeof(struct nd_router_advert);
    va_list vArg;
    va_start(vArg, optionsNum);
    const ndp_optPayload *optionsList[optionsNum];

    for (int i = 0; i < optionsNum; ++i) {
        const ndp_optPayload *opt = va_arg(vArg, const ndp_optPayload *);
        optionsList[i] = opt;
        length += opt->Length * 8;
    }

    struct nd_router_advert *pkt = malloc(length);
    pkt->nd_ra_hdr.icmp6_type = ND_ROUTER_ADVERT;
    pkt->nd_ra_hdr.icmp6_code = 0;
    pkt->nd_ra_hdr.icmp6_dataun.icmp6_un_data8[0] = curHopLimit;    // Cur Hop Limit

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    pkt->nd_ra_hdr.icmp6_dataun.icmp6_un_data8[1] =
            ndp_ra_flag_Prf_Medium & (ndp_ra_flag_R);
#pragma clang diagnostic pop
    pkt->nd_ra_hdr.icmp6_dataun.icmp6_un_data16[1] = htobe16(routerLifeTime);
    pkt->nd_ra_reachable = htobe32(reachableTime);
    pkt->nd_ra_retransmit = htobe32(reTransTimer);

    ndp_optPayload *options = (ndp_optPayload *) ((uint8_t *) pkt + sizeof(struct nd_router_advert));
    for (int i = 0; i < optionsNum; ++i) {
        uint optionLength = optionsList[i]->Length * 8;
        memcpy(options, optionsList[i], optionLength);
        options = (ndp_optPayload *) ((uintptr_t) options + optionLength);
    }

    // checksum
    if (ndp_checksum(&pkt->nd_ra_hdr.icmp6_cksum, sourceAddr, destAddr, pkt, length) < 0) {
        // error
    }
    *pktBufPtr = pkt;
    return length;
}

struct ndp_pseudoHeader {
    struct in6_addr SourceAddress;
    struct in6_addr DestinationAddress;
    uint32_t UpperLayerPacketLength;
    uint8_t Zero[3];
    uint8_t NextHeader;
} __attribute__((packed));

int ndp_checksumCalCore(int *buf, const uint16_t *data, const size_t size) {
    if (size & 1)
        return -1;

    int sum = 0;
    for (int i = 0; i < size / 2; ++i) {
        sum += data[i];

        if (sum & 0xFFFF0000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
    }

    *buf = sum;
    return 0;
}

int ndp_checksum(
        uint16_t *buf,
        const struct in6_addr sourceAddr, const struct in6_addr destAddr,
        struct nd_router_advert *restrict packet,
        const size_t size) {
    if (packet == NULL || size == 0)
        return -1;

    packet->nd_ra_hdr.icmp6_cksum = 0x00;

    struct ndp_pseudoHeader header = {
            .SourceAddress = sourceAddr,
            .DestinationAddress = destAddr,
            .UpperLayerPacketLength = size,
            .Zero = {0x00, 0x00, 0x00},
            .NextHeader = IPPROTO_ICMPV6
    };

    const uint16_t *headerPtr = (const uint16_t *) &header;

    int sumHeader = 0;
    int sumBody = 0;

    if (ndp_checksumCalCore(&sumHeader, headerPtr, sizeof(header)) < 0) {
        // error
    }
    if (ndp_checksumCalCore(&sumBody, (const uint16_t *) packet, size) < 0) {
        // error
    }

    return ~(sumHeader + sumBody) & 0xFFFF;
}

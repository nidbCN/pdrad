#include <stdlib.h>
#include <string.h>
#include "ndp_packets.h"
#include "ndp_options.h"

ndp_ra *ndp_ra_createPacket(
        struct in6_addr sourceAddr,
        struct in6_addr destAddr,
        uint8_t curHopLimit,
        uint16_t routerLifeTime,
        uint32_t reachableTime,
        uint32_t reTransTimer,
        ndp_optPayload *optionsList[],
        uint8_t optionsNum
) {
    size_t length = sizeof(ndp_ra);

    for (int i = 0; i < optionsNum; ++i) {
        length += optionsList[i]->Length * 8;
    }

    ndp_ra *pkt = (ndp_ra *) malloc(length);
    pkt->Type = 134;
    pkt->Code = 0;
    pkt->CurHopLimit = curHopLimit;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    pkt->Flags = (!M) | (!O) & (0xC0);
#pragma clang diagnostic pop
    pkt->RouterLifetime = routerLifeTime;
    pkt->ReachableTime = reachableTime;
    pkt->ReTransTimer = reTransTimer;

    ndp_optPayload *options = (ndp_optPayload *) pkt->Options;
    for (int i = 0; i < optionsNum; ++i) {
        uint optionLength = optionsList[i]->Length * 8;
        memcpy(options, optionsList[i], optionLength);
        options = (ndp_optPayload *) ((uintptr_t) options + optionLength);
    }

    // checksum
    int sum = ndp_checksum(sourceAddr, destAddr, pkt, length);
    pkt->CheckSum = sum;

    return pkt;
}

struct ndp_pseudoHeader {
    struct in6_addr SourceAddress;
    struct in6_addr DestinationAddress;
    uint32_t UpperLayerPacketLength;
    uint8_t Zero[3];
    uint8_t NextHeader;
} __attribute__((packed));

uint16_t ndp_checksumCalCore(const uint16_t *data, size_t size) {
    int sum = 0;
    for (int i = 0; i < size / 2; ++i) {
        sum += data[i];

        if (sum & 0xFFFF0000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
    }

    if (size & 1) {
        uint16_t final_byte = ((const uint8_t *) data)[size - 1] << 8;
        sum += final_byte;
        if (sum & 0xFFFF0000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
    }

    return sum;
}

uint16_t ndp_checksum(struct in6_addr sourceAddr, struct in6_addr destAddr, ndp_ra *restrict packet, size_t size) {
    if (packet == NULL || size == 0)
        return 0;

    packet->CheckSum = 0x00;

    struct ndp_pseudoHeader header = {
            .SourceAddress = sourceAddr,
            .DestinationAddress = destAddr,
            .UpperLayerPacketLength = size,
            .Zero = {0x00, 0x00, 0x00},
            .NextHeader= 58
    };

    const uint16_t *headerPtr = (const uint16_t *) &header;
    int sum = ndp_checksumCalCore(headerPtr, sizeof(header)) +
              ndp_checksumCalCore((const uint16_t *) packet, size);

    return ~sum & 0xFFFF;
}

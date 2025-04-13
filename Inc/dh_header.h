#ifndef PDRAD_DH_HEADER_H
#define PDRAD_DH_HEADER_H

#include <stdint.h>
#include <stdbool.h>
#include "dh_options.h"

#define DH_ALL_DHCP_RELAY_AGENTS_AND_SERVERS ("ff02::1:2")

typedef struct _dh_pkt_TransactionId {
    uint8_t TransactionId_0;
    uint8_t TransactionId_1;
    uint8_t TransactionId_2;
}  __attribute__((packed)) dh_pkt_TransactionId;

#define dh_pkt_offset (sizeof(uint8_t) + sizeof(dh_pkt_TransactionId))

typedef struct _dh_pkt {
    uint8_t MsgType;
    dh_pkt_TransactionId TransactionId;
    dh_optPayload Options[];
} __attribute__((packed)) dh_pkt;

enum dh_msgType {
    dh_SOLICIT = 1,
    dh_ADVERTISE = 2,
    dh_REQUEST = 3,
    dh_CONFIRM = 4,
    dh_RENEW = 5,
    dh_REBIND = 6,
    dh_REPLY = 7,
    dh_RELEASE = 8,
    dh_DECLINE = 9,
    dh_RECONFIGURE = 10,
    dh_INFORMATION_REQUEST = 11,
    dh_RELAY_FORW = 12,
    dh_RELAY_REPL = 13
};

size_t dh_createPacket(const dh_pkt **pktPtr, enum dh_msgType msgType, dh_pkt_TransactionId transId,
                       const dh_optPayload *optPtrList[], uint optPtrListLength);

size_t dh_createSolicitPacket(const dh_pkt **pktPtr, const dh_opt_ClientIdentifier *clientId, size_t clientSize,
                              uint16_t elapsedTime,
                              uint32_t IA_Id);

size_t dh_createRapidSolicitPacket(const dh_pkt **pktPtr, const dh_opt_ClientIdentifier *clientId, size_t clientSize,
                                   uint16_t elapsedTime,
                                   uint32_t IA_Id);

size_t dh_CreateCustomizedSolicitPacket(
        const dh_pkt **pktPtr,
        const dh_optPayload *clientId,
        const dh_optPayload *elapsedTime,
        const dh_optPayload *IA_PD,
        uint optionsNum, ...);

size_t dh_createCustomizedRequestPacket(const dh_pkt **pktPtr, const dh_optPayload *clientId, uint16_t elapsedTime,
                                        uint32_t IA_Id);

// Read options from DHCPv6 packet
// pkt: a pure DHCPv6 packet, do not contain UDP protocol
// size: DHCPv6 packet length
// NOTE: do not free *pkt, it's used in dh_parsedOptions
dh_parsedOptions dh_parseOptions(const dh_pkt *pkt, size_t size);

#endif //PDRAD_DH_HEADER_H

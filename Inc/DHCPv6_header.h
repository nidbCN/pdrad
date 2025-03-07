//
// Created by nidb on 3/3/25.
//

#ifndef KEADYNPREFIX_DHCPV6_HEADER_H
#define KEADYNPREFIX_DHCPV6_HEADER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct _DHCPv6_pkt {
    uint8_t MsgType;
    uint8_t TransactionId[3];
    uint8_t Options[];
} __attribute__((packed)) DHCPv6_pkt;

enum DHCPv6_msgType {
    SOLICIT = 1,
    ADVERTISE = 2,
    REQUEST = 3,
    CONFIRM = 4,
    RENEW = 5,
    REBIND = 6,
    REPLY = 7,
    RELEASE = 8,
    DECLINE = 9,
    RECONFIGURE = 10,
    INFORMATION_REQUEST = 11,
    RELAY_FORW = 12,
    RELAY_REPL = 13
};

bool DHCPv6_isCSMessage(uint16_t msg_type);

#endif //KEADYNPREFIX_DHCPV6_HEADER_H

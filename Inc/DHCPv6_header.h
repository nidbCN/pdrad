#ifndef PDRAD_DHCPV6_HEADER_H
#define PDRAD_DHCPV6_HEADER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct _dh_pkt {
    uint8_t MsgType;
    uint8_t TransactionId[3];
    uint8_t Options[];
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

#endif //PDRAD_DHCPV6_HEADER_H

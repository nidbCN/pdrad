#ifndef PDRAD_NDP_PACKETS_H
#define PDRAD_NDP_PACKETS_H

#include <stdint.h>

typedef struct _ndp_ra {
    uint8_t Type;
    uint8_t Code;
    uint16_t CheckSum;
    uint8_t CurHopLimit;
    uint8_t Flags;
    uint16_t RouterLifetime;
    uint32_t ReachableTime;
    uint32_t ReTransTimer;
    uint8_t Options[];
} __attribute__((packed)) ndp_ra;

enum ndp_ra_flag {
    M = 0x80,
    O = 0x40,
    PrfHigh = 0x08,
    PrfMedium = 0x00,
    PrfLow = 0x18,
    PrfInvalid = 0x10,
    Reserved = 0x00
};

#endif //PDRAD_NDP_PACKETS_H

#ifndef PDRAD_NDP_PACKETS_H
#define PDRAD_NDP_PACKETS_H

#include <netinet/in.h>
#include "ndp_options.h"

typedef struct _ndp_ra {
    uint8_t Type;
    uint8_t Code;
    uint16_t CheckSum;
    uint8_t CurHopLimit;
    uint8_t Flags;
    uint16_t RouterLifetime;
    uint32_t ReachableTime;
    uint32_t ReTransTimer;
    ndp_optPayload Options[];
} __attribute__((packed)) ndp_ra;

typedef struct _ndp_rs {
    uint8_t Type;
    uint8_t Code;
    uint16_t CheckSum;
    uint32_t Reserved;
    ndp_optPayload Options[];
} ndp_rs;

// @formatter:off
// clang-format off

/* Router Advertisement Flags
 * NOTE: must use `& ndp_ra_flag_R` before send
 * REF: [RFC5175](https://www.rfc-editor.org/rfc/rfc5175.html)
 */
enum ndp_ra_flag {
    /* Managed Address Configuration Flag
     * REF: [RFC4861](https://www.rfc-editor.org/rfc/rfc4861.html#section-4.2)
     */
    ndp_ra_flag_M =             0b10000000,
    /* Other Configuration Flag
     * REF: [RFC4861](https://www.rfc-editor.org/rfc/rfc4861.html#section-4.2)
     */
    ndp_ra_flag_O =             0b01000000,
    /* Mobile IPv6 Home Agent Flag
     * REF: [RFC3775](https://www.rfc-editor.org/rfc/rfc3775.html#section-7.1)
     */
    ndp_ra_flag_H =             0b00100000,
    /* Router Selection Preferences MEDIUM
     * Indicates whether to prefer this router over other default routers.
     * If the Router Lifetime is zero, the preference value MUST be set to (00) by the sender and MUST be ignored by the receiver.
     * REF: [RFC4191](https://www.rfc-editor.org/rfc/rfc4191.html#section-2.2)
     */
    ndp_ra_flag_Prf_Medium =    0b00000000,
    /* Router Selection Preferences HIGH
     * Indicates whether to prefer this router over other default routers.
     * REF: [RFC4191](https://www.rfc-editor.org/rfc/rfc4191.html#section-2.2)
     */
    ndp_ra_flag_Prf_High =      0b00001000,
    /* Router Selection Preferences LOW
     * Indicates whether to prefer this router over other default routers.
     * REF: [RFC4191](https://www.rfc-editor.org/rfc/rfc4191.html#section-2.2)
     */
    ndp_ra_flag_Prf_Low =       0b00011000,
    /* Router Selection Preferences INVALID
     * Indicates whether to prefer this router over other default routers.
     * If the Reserved(10) value is received, the receiver MUST treat the value as if it were (00).
     * REF: [RFC4191](https://www.rfc-editor.org/rfc/rfc4191.html#section-2.2)
     */
    ndp_ra_flag_Invalid =       0b00010000,
    /* Neighbor Discovery Proxy Flag        [RFC4389]
     * REF: [RFC4389](https://www.rfc-editor.org/rfc/rfc4389.html#section-4.1.3.3)
     */
    ndp_ra_flag_P =             0b00000100,
    /* Reserved */
    ndp_ra_flag_R =             0b11111100
};

// @formatter:on
// clang-format on

uint16_t ndp_checksum(struct in6_addr sourceAddr, struct in6_addr destAddr, ndp_ra *restrict packet, size_t size);

ndp_ra *ndp_ra_createPacket(
    struct in6_addr sourceAddr,
    struct in6_addr destAddr,
    uint8_t curHopLimit,
    uint16_t routerLifeTime,
    uint32_t reachableTime,
    uint32_t reTransTimer,
    ndp_optPayload *optionsList[],
    uint8_t optionsNum
);

#endif // PDRAD_NDP_PACKETS_H

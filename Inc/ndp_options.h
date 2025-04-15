#ifndef PDRAD_NDP_OPTIONS_H
#define PDRAD_NDP_OPTIONS_H

#include <stdint.h>
#include <netinet/in.h>

typedef struct _ndp_optPayload {
    uint8_t Type;
    uint8_t Length; // include type and length, units in 8bytes/64bits
    uint8_t OptionData[];
} __attribute__((packed)) ndp_optPayload;

#define ndp_optOffset (sizeof(uint8_t) + sizeof(uint8_t))

typedef struct _ndp_opt_LinkLayerAddress {
    uint8_t EtherAddress[6];
} __attribute__((packed)) ndp_opt_LinkLayerAddress;

typedef struct _ndp_opt_PrefixInformation {
    uint8_t PrefixLength;
    uint8_t Flags;
    uint32_t ValidLifetime;
    uint32_t PreferredLifetime;
    uint32_t Reserved;
    uint64_t Prefix[2];
} __attribute__((packed)) ndp_opt_PrefixInformation;

enum ndp_opt_PrefixInformationFlag {
    L = 0x80,
    A = 0x40,
};

typedef struct _ndp_opt_Mtu {
    uint16_t Reserved;
    uint32_t Mtu;
} __attribute__((packed)) ndp_opt_Mtu;


ndp_optPayload *ndp_createOptionSourceLinkLayerAddress(uint8_t etherAddr[6]);

ndp_optPayload *ndp_createOptionTargetLinkLayerAddress(uint8_t etherAddr[6]);

ndp_optPayload *ndp_createOptionPrefixInformation(
        uint8_t prefixLength,
        enum ndp_opt_PrefixInformationFlag flags,
        uint32_t validLifetime,
        uint32_t preferredLifetime,
        struct in6_addr prefix);

ndp_optPayload *ndp_createOptionMtu(uint32_t mtu);

#endif //PDRAD_NDP_OPTIONS_H

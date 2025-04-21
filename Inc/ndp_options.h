#ifndef PDRAD_NDP_OPTIONS_H
#define PDRAD_NDP_OPTIONS_H

#include <stdint.h>
#include <netinet/in.h>

typedef struct _ndp_optPayload
{
    uint8_t Type;
    uint8_t Length; // include type and length, units in 8bytes/64bits
    uint8_t OptionData[];
} __attribute__((packed)) ndp_optPayload;

#define ndp_optOffset (sizeof(uint8_t) + sizeof(uint8_t))

typedef struct _ndp_opt_LinkLayerAddress
{
    uint8_t EtherAddress[6];
} __attribute__((packed)) ndp_opt_LinkLayerAddress;

typedef struct _ndp_opt_PrefixInformation
{
    uint8_t PrefixLength;
    uint8_t Flags;
    uint32_t ValidLifetime;
    uint32_t PreferredLifetime;
    uint32_t Reserved;
    uint64_t Prefix[2];
} __attribute__((packed)) ndp_opt_PrefixInformation;

enum ndp_opt_PrefixInformationFlag
{
    ndp_opt_PrefixInformation_flag_L = 0b10000000,
    ndp_opt_PrefixInformation_flag_A = 0b01000000,
    ndp_opt_PrefixInformation_flag_R = 0b11000000,
};

typedef struct _ndp_opt_Mtu
{
    uint16_t Reserved;
    uint32_t Mtu;
} __attribute__((packed)) ndp_opt_Mtu;

/*
Advertisement Interval Option
* rfc: https://www.rfc-editor.org/rfc/rfc6275.html#section-7.3
*/
typedef struct _ndp_opt_AdvertisementIntervalOption
{
    uint16_t Reserved;
#define NDP_OPT_ADVERTISEMENT_INTERVAL_PROP_TYPE (7)
#define NDP_OPT_ADVERTISEMENT_INTERVAL_PROP_LEN (1)
    /*
    The maximum time, in milliseconds,between successive unsolicited Router Advertisement messages sent by this router on this network interface.
    Using the conceptual router configuration variables defined by Neighbor Discovery [18], this field MUST be equal to the value MaxRtrAdvInterval, expressed in milliseconds.
    */
    uint32_t AdvertisementInterval;
} __attribute__((packed)) ndp_opt_AdvertisementIntervalOption;

typedef struct _ndp_opt_RouteInformationOption
{
#define NDP_OPT_ROUTE_INFORMATION_PROP_TYPE (24)
    /*
    The length of the option(including the Type and Length fields) in units of 8 octets.
    * The Length field is 1, 2, or 3 depending on the Prefix Length.
    * If Prefix Length is greater than 64, then Length must be 3.
    * If Prefix Length is greater than 0, then Length must be 2 or 3.
    * If Prefix Length is zero, then Length must be 1, 2, or 3.
    */
#define NDP_OPT_ROUTE_INFORMATION_PROP_LEN (1)
    /*
    The number of leading bits in the Prefix that are valid.
    * The value ranges from 0 to 128.
    * The Prefix field is 0, 8, or 16 octets depending on Length.
    */
    uint8_t PrefixLength;
    /*
    The Route Preference indicates whether to prefer the router associated with this prefix over others, when multiple identical prefixes (for different routers) have been received.
    * If the Reserved (10) value is received, the Route Information Option MUST be ignored.
    * NOTE: 2bits, offset 3bits. 0b10(0b000_10_000 in byte) for reserved.
    */
    uint8_t RoutePreference;
    /*
    The length of time in seconds (relative to the time the packet is sent) that the prefix is valid for route determination.
    * A value of all one bits (0xffffffff) represents infinity.
    */
    uint32_t RouteLifetime;
    /* Variable-length field containing an IP address or a prefix of an IP address.
     * The Prefix Length field contains the number of valid leading bits in the prefix.
     * The bits in the prefix after the prefix length (if any) are reserved and MUST be initialized to zero by the sender and ignored by the receiver.
     * NOTE: only vaild address and packed to 32bits. for example, `/64` use the first 4bytes and `/56` use the first 4bytes with the last 8 bits be filled with 0x0.
     */
    uint32_t Perfix[];
} ndp_opt_RouteInformation;

ndp_optPayload *ndp_createOptionSourceLinkLayerAddress(uint8_t etherAddr[6]);

ndp_optPayload *ndp_createOptionTargetLinkLayerAddress(uint8_t etherAddr[6]);

ndp_optPayload *ndp_createOptionPrefixInformation(
    uint8_t prefixLength,
    enum ndp_opt_PrefixInformationFlag flags,
    uint32_t validLifetime,
    uint32_t preferredLifetime,
    struct in6_addr prefix);

ndp_optPayload *ndp_createOptionMtu(uint32_t mtu);

typedef struct _ndp_parsedOptions
{
    ndp_opt_LinkLayerAddress SourceLinkLayerAddress;
    ndp_opt_LinkLayerAddress TargetLinkLayerAddress;
    ndp_opt_PrefixInformation PrefixInformation;
    ndp_opt_Mtu Mtu;
} ndp_parsedOptions;

#endif // PDRAD_NDP_OPTIONS_H

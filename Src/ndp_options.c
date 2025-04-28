#include <string.h>
#include <stdlib.h>
#include "ndp_options.h"
#include "global.h"

ndp_optPayload *ndp_createOptionLinkLayerAddressCore(const uint8_t type, uint8_t etherAddr[6]) {
    if (type != 1 && type != 2)
        return NULL;

    ndp_optPayload *option = malloc(ndp_optOffset + 6);
    option->Type = type;
    option->Length = 1;

    ndp_opt_LinkLayerAddress *data = (ndp_opt_LinkLayerAddress *) &(option->OptionData);

    memcpy(data->EtherAddress, etherAddr, 6);

    return option;
}

ndp_optPayload *ndp_createOptionSourceLinkLayerAddress(uint8_t etherAddr[6]) {
    return ndp_createOptionLinkLayerAddressCore(1, etherAddr);
}

ndp_optPayload *ndp_createOptionTargetLinkLayerAddress(uint8_t etherAddr[6]) {
    return ndp_createOptionLinkLayerAddressCore(2, etherAddr);
}

ndp_optPayload *ndp_createOptionPrefixInformation(
        const uint8_t prefixLength,
        const enum ndp_opt_PrefixInformationFlag flags,
        const uint32_t validLifetime,
        const uint32_t preferredLifetime,
        struct in6_addr prefix) {
    ndp_optPayload *option = malloc(ndp_optOffset + sizeof(ndp_opt_PrefixInformation));
    option->Type = 3;
    option->Length = 4;

    ndp_opt_PrefixInformation *data = (ndp_opt_PrefixInformation *) &(option->OptionData);

    data->PrefixLength = prefixLength;
    data->Flags = (uint8_t) (flags & 0xC0);
    data->ValidLifetime = htobe32(validLifetime);
    data->PreferredLifetime = htobe32(preferredLifetime);
    data->Reserved = 0x00;

    uint64_t *addrPtr = (uint64_t *) (&prefix);
    data->Prefix[0] = htobe64(addrPtr[0]);
    data->Prefix[1] = htobe64(addrPtr[1]);

    return option;
}

ndp_optPayload *ndp_createOptionMtu(const uint32_t mtu) {
    ndp_optPayload *option = malloc(ndp_optOffset + sizeof(ndp_opt_Mtu));
    option->Type = 5;
    option->Length = 1;

    ndp_opt_Mtu *data = (ndp_opt_Mtu *) &(option->OptionData);

    data->Reserved = 0x00;
    data->Mtu = htobe32(mtu);

    return option;
}

ndp_optPayload *ndp_createOptionAdvertisementIntervalOption(const uint32_t interval) {
    ndp_optPayload *option = malloc(NDP_OPT_ADVERTISEMENT_INTERVAL_PROP_LEN * 8);
    ndp_opt_AdvertisementIntervalOption *subOption = (ndp_opt_AdvertisementIntervalOption *) option->OptionData;
    subOption->AdvertisementInterval = htobe32(interval);
    return option;
}

ndp_optPayload *ndp_createOptionRouteInformation(
        const uint8_t prefixLength,
        uint8_t routePreference,
        const uint32_t routeLifetime,
        struct in6_addr prefix) {
    size_t propLen = NDP_OPT_ROUTE_INFORMATION_PROP_LEN;

    if (prefixLength > 128) {
        // invalid arguments
    }

    if (prefixLength != 0) {
        propLen += prefixLength / 64; // length in 8 bytes
    }

    ndp_optPayload *option = malloc(propLen * 8);

    option->Type = 24;
    option->Length = propLen;

    ndp_opt_RouteInformation *subOption = (ndp_opt_RouteInformation *) option->OptionData;

    subOption->PrefixLength = prefixLength;
    subOption->RoutePreference = routePreference;
    subOption->RouteLifetime = htobe32(routeLifetime);

    if (prefixLength == 0)
        return option;

    htobe_inet6(prefix);
    if (prefixLength >= 64) {
        ((uint64_t *) &prefix)[1] = (((uint64_t *) &prefix)[1] << (128 - prefixLength)) >> (128 - prefixLength);
    }
    if (prefixLength < 64) {
        ((uint64_t *) &prefix)[0] = (((uint64_t *) &prefix)[0] << (64 - prefixLength)) >> (64 - prefixLength);
    }

    memcpy(subOption->Perfix, &prefix, (propLen - 1) * 8);

    return option;
}
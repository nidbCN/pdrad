#include <string.h>
#include <stdlib.h>
#include "ndp_options.h"

ndp_optPayload *ndp_createOptionLinkLayerAddressCore(uint8_t type, uint8_t etherAddr[6]) {
    if (type != 1 && type != 2)
        return NULL;

    ndp_optPayload *option = (ndp_optPayload *) malloc(ndp_optOffset + 6);
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
        uint8_t prefixLength,
        enum ndp_opt_PrefixInformationFlag flags,
        uint32_t validLifetime,
        uint32_t preferredLifetime,
        struct in6_addr prefix) {
    ndp_optPayload *option = (ndp_optPayload *) malloc(ndp_optOffset + sizeof(ndp_opt_PrefixInformation));
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

ndp_optPayload *ndp_createOptionMtu(uint32_t mtu) {
    ndp_optPayload *option = (ndp_optPayload *) malloc(ndp_optOffset + sizeof(ndp_opt_Mtu));
    option->Type = 5;
    option->Length = 1;

    ndp_opt_Mtu *data = (ndp_opt_Mtu *) &(option->OptionData);

    data->Reserved = 0x00;
    data->Mtu = htobe32(mtu);

    return option;
}
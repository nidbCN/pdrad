#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "dh_options.h"
#include "global.h"
#include <netinet/in.h>

// optionDataLength without metadata
dh_optPayload *dh_createCustomOptPayload(const uint16_t optionCode, const void *optionData, const size_t optionDataLength) {
    size_t size = optionDataLength + dh_optPayload_offset;

    dh_optPayload *payload = (dh_optPayload *) malloc(size);
    payload->OptionCode = optionCode;
    payload->OptionLength = optionDataLength;

    if (optionData != NULL)
        memcpy(payload->OptionData, optionData, optionDataLength);

    return payload;
}

dh_optPayload *dh_createOptPayload(const enum dh_options optionCode, const void *optionData, const size_t optionDataLength) {
    return dh_createCustomOptPayload(optionCode, optionData, optionDataLength);
}

dh_optPayload *dh_createOption_IA_PD(const uint32_t id, const dh_optPayload prefixOptions[], const size_t prefixOptionsLength) {
    dh_optPayload *payload = malloc(sizeof(dh_opt_IA_PD) + dh_optPayload_offset);
    dh_opt_IA_PD *thisOption = (dh_opt_IA_PD *) (payload->OptionData);

    thisOption->IA_id = htobe32(id);
    thisOption->Time1 = 0x00;
    thisOption->Time2 = 0x00;

    payload->OptionCode = DHCPv6_OPTION_IA_PD;
    if (prefixOptions != NULL) {
        memcpy(thisOption->Options, prefixOptions, prefixOptionsLength);
        payload->OptionLength = sizeof(dh_opt_IA_PD) + prefixOptionsLength;
    } else {
        payload->OptionLength = sizeof(dh_opt_IA_PD);
    }

    return payload;
}

dh_optPayload *
dh_createOption_IAPrefix(const uint32_t preferredTime, const uint32_t validTime, const uint8_t prefixLength, const struct in6_addr prefix) {
    uint optionLength = sizeof(dh_opt_IA_Prefix) - sizeof(uint8_t);

    dh_optPayload *payload = (dh_optPayload *) malloc(optionLength + dh_optPayload_offset);
    payload->OptionCode = DHCPv6_OPTION_IAPREFIX;
    payload->OptionLength = optionLength;

    dh_opt_IA_Prefix *option = (dh_opt_IA_Prefix *) (payload->OptionData);

    option->PreferredLifetime = htobe16(preferredTime);
    option->ValidLifetime = htobe16(validTime);
    option->PrefixLength = prefixLength;
    memcpy(&(option->Prefix), &prefix, sizeof(struct in6_addr));

    return payload;
}

dh_optPayload *dh_createOption_RapidCommit() {
    return dh_createOptPayload(DHCPv6_OPTION_RAPID_COMMIT, NULL, 0);
}

dh_optPayload *dh_createOption_ClientIdentifier_En(const uint32_t enterpriseNumber, uint8_t id[], const size_t idLength) {
    if (idLength <= 0)
        return NULL;

    dh_optPayload *payload = (dh_optPayload *) malloc(
            dh_optPayload_offset + sizeof(uint16_t) + sizeof(uint32_t) + idLength);
    payload->OptionCode = DHCPv6_OPTION_CLIENTID;
    payload->OptionLength = sizeof(uint16_t) + sizeof(uint32_t) + idLength;

    dh_opt_ClientIdentifier *option = (dh_opt_ClientIdentifier *) (payload->OptionData);

    option->DUIdType = htobe16(DUId_En);
    option->DUIdData.DUId_En.EnterpriseNumber = htobe32(enterpriseNumber);
    memcpy(option->DUIdData.DUId_En.Identifier, id, idLength);

    return payload;
}

// unit: 1/100 sec
dh_optPayload *dh_createOption_ElapsedTime(const uint16_t time) {
    dh_optPayload *payload = malloc(dh_optPayload_offset + sizeof(dh_opt_ElapsedTime));
    payload->OptionCode = DHCPv6_OPTION_ELAPSED_TIME;
    payload->OptionLength = sizeof(dh_opt_ElapsedTime);

    dh_opt_ElapsedTime *option = (dh_opt_ElapsedTime *) (payload->OptionData);
    option->ElapsedTime = htobe16(time);

    return payload;
}

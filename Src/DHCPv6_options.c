#include <stdlib.h>
#include <string.h>
#include "DHCPv6_options.h"
#include "DHCPv6_header.h"
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

bool
createCustomOptionPayload(DHCPv6_optPayload *payload, uint16_t optionCode, void *optionData, size_t optionDataLength) {
    payload->OptionCode = htobe16(optionCode);
    payload->OptionLength = htobe16(optionDataLength);

    if (optionDataLength != 0 && optionData != NULL)
        memcpy(payload->OptionData, optionData, optionDataLength);

    return payload != NULL;
}

// optionDataLength in bytes
bool
createOptionPayload(DHCPv6_optPayload *payload, enum DHCPv6_optCode optionCode, void *optionData,
                    size_t optionDataLength) {
    return createCustomOptionPayload(payload, optionCode, optionData, optionDataLength);
}

bool createOption_IA_PD(DHCPv6_optPayload *payload, uint32_t id, uint32_t preferredRenewalTimeSec,
                        uint32_t preferredRebindTime) {
    if (payload == NULL)
        return false;

    DHCPv6_opt_IA_PD *option = (DHCPv6_opt_IA_PD *) malloc(sizeof(DHCPv6_opt_IA_PD));
    option->IA_id = htobe32(id);
    option->Time1 = htobe32(preferredRenewalTimeSec);
    option->Time2 = htobe32(preferredRebindTime);
    bool result = createOptionPayload(payload, DHCPv6_OPTION_IA_PD, option, sizeof(DHCPv6_opt_IA_PD));
    free(option);
    return result;
}

bool createOption_RapidCommit(DHCPv6_optPayload *payload) {
    if (payload == NULL)
        return false;

    bool result = createOptionPayload(payload, DHCPv6_OPTION_RAPID_COMMIT, NULL, 0);
    return result;
}

bool
createOption_ClientIdentifier_En(DHCPv6_optPayload *payload, uint32_t enterpriseNumber, uint8_t id[], size_t idLength) {
    if (payload == NULL)
        return false;

    DHCPv6_opt_ClientIdentifier *option = (DHCPv6_opt_ClientIdentifier *) malloc(idLength + 6);

    option->DUIdType = htobe16(DUId_En);
    option->Data.DUId_En.EnterpriseNumber = htobe32(enterpriseNumber);
    memcpy(option->Data.DUId_En.Identifier, id, idLength);

    bool result = createOptionPayload(payload, DHCPv6_OPTION_CLIENTID, option, idLength + 6);
    free(option);
    return result;
}

bool createOption_ElapsedTime(DHCPv6_optPayload *payload, uint16_t time) {
    if (payload == NULL)
        return false;

    DHCPv6_opt_ElapsedTime *option = (DHCPv6_opt_ElapsedTime *) malloc(sizeof(DHCPv6_opt_ElapsedTime));
    option->ElapsedTime = htobe16(time);
    bool result = createOptionPayload(payload, DHCPv6_OPTION_ELAPSED_TIME, option, sizeof(DHCPv6_opt_ElapsedTime));
    free(option);
    return result;
}

// offset in byte
int readOptionFromCSMessage(DHCPv6_optPayload *buffer, DHCPv6_pkt *pkt, size_t length, int index, int offset) {
    if (pkt == NULL) return EFAULT;
    if (!DHCPv6_isCSMessage(pkt->MsgType)) return EINVAL;

    int i = 0;

    DHCPv6_optPayload *payload = NULL;

    // skip
    while (i != index) {
        payload = (DHCPv6_optPayload *) &pkt->Options[offset];
        offset += (payload->OptionLength + (16 + 16));

        if (offset >= length)
            return ENOENT;

        printf("skip option index %d, with option-code %d, len %d, now offset %d.\n",
               i,
               payload->OptionCode,
               payload->OptionLength,
               offset);

        i++;
    }

    payload = (DHCPv6_optPayload *) &pkt->Options[offset];
    int resultLength = payload->OptionLength + (16 + 16);

    printf("success read option index %d, with option-code %d, len %d.",
           i,
           payload->OptionCode,
           payload->OptionLength);

    memcpy(buffer, payload, resultLength);
    return resultLength;
}

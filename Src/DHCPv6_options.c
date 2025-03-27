#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "DHCPv6_options.h"
#include "global.h"

// optionDataLength without metadata
dh_optPayload *dh_createCustomOptPayload(uint16_t optionCode, void *optionData, size_t optionDataLength) {
    if (optionDataLength <= 0)
        return NULL;

    dh_optPayload *payload = (dh_optPayload *) malloc(optionDataLength + 4);
    payload->OptionCode = optionCode;
    payload->OptionLength = optionDataLength;

    if (optionData != NULL)
        memcpy(payload->OptionData, optionData, optionDataLength);

    return payload;
}

dh_optPayload *dh_createOptPayload(enum dh_options optionCode, void *optionData, size_t optionDataLength) {
    return dh_createCustomOptPayload(optionCode, optionData, optionDataLength);
}

// NOTE: preferredRenewalTimeSec and preferredRebindTime should be 0x00 in client message
dh_optPayload *dh_createOption_IA_PD(uint32_t id, uint32_t preferredRenewalTimeSec, uint32_t preferredRebindTime) {
    // TODO: option-len is 12 + length of IA_PD-options field.

    dh_optPayload *payload = alloca(sizeof(dh_opt_IA_PD) + dh_optPayload_offset);
    dh_opt_IA_PD *option = (dh_opt_IA_PD *) (payload + dh_optPayload_offset);

    option->IA_id = htobe32(id);
    option->Time1 = htobe32(preferredRenewalTimeSec);
    option->Time2 = htobe32(preferredRebindTime);

    payload->OptionCode = DHCPv6_OPTION_IA_PD;
    payload->OptionLength = sizeof(dh_opt_IA_PD);

    return payload;
}

dh_optPayload *dh_createOption_RapidCommit() {
    return dh_createOptPayload(DHCPv6_OPTION_RAPID_COMMIT, NULL, 0);
}

dh_optPayload *dh_createOption_ClientIdentifier_En(uint32_t enterpriseNumber, uint8_t id[], size_t idLength) {
    if (idLength <= 0)
        return NULL;

    dh_opt_ClientIdentifier *option = (dh_opt_ClientIdentifier *) malloc(idLength + 6);

    option->DUIdType = htobe16(DUId_En);
    option->Data.DUId_En.EnterpriseNumber = htobe32(enterpriseNumber);
    memcpy(option->Data.DUId_En.Identifier, id, idLength);

    bool result = dh_createOptPayload(payload, DHCPv6_OPTION_CLIENTID, option, idLength + 6);
    free(option);
    return result;
}

bool dh_createOption_ElapsedTime(dh_optPayload *payload, uint16_t time) {
    if (payload == NULL)
        return false;

    dh_opt_ElapsedTime *option = new(dh_opt_ElapsedTime);
    option->ElapsedTime = htobe16(time);
    bool result = dh_createOptPayload(payload, DHCPv6_OPTION_ELAPSED_TIME, option, sizeof(dh_opt_ElapsedTime));
    free(option);
    return result;
}

// do not free *pkt, it's used in dh_parsedOptions
dh_parsedOptions dh_parseOptions(const DHCPv6_pkt *pkt, size_t size) {
    log_debug("Start parse option, address: %p, length: %d.", pkt, size);

    dh_optPayload *readerPtr = (dh_optPayload *) (pkt + sizeof(uint16_t) + sizeof(uint16_t));
    size_t unreadDataLength = size - (sizeof(uint16_t) + sizeof(uint16_t));

    log_trace("Skip header, address: %p, length: %d.", readerPtr, unreadDataLength);

    dh_parsedOptions result = {.success = false};

    // invalid status
    if (unreadDataLength <= 0 || readerPtr == NULL) {
        log_error("Invalid packet with content length %d and address %p.", unreadDataLength, readerPtr);
        return result;
    }

    // invalid type
    if (pkt->MsgType != ADVERTISE) {
        log_error("Wrong message type %d, expect ADVERTISE.", pkt->MsgType);
        return result;
    }

    while (unreadDataLength > 0) {
        dh_optPayload *option = readerPtr;
        log_debug("Start read option at %p(offset %d bytes).", option, (void *) option - (void *) pkt);

        switch (be16toh(option->OptionCode)) {
            case DHCPv6_OPTION_CLIENTID:
                result.ClientIdentifier = option;
                break;
            case DHCPv6_OPTION_IA_PD:
                result.IA_PD = option;
                break;
            case DHCPv6_OPTION_IAPREFIX:
                result.success = false;
                dh_optMultiPayload *newNode = new(dh_optMultiPayload);
                newNode->value = option;

                if (result.IA_PrefixList != NULL) {
                    result.IA_PrefixList = newNode;
                } else {
                    dh_optMultiPayload *currentNode = result.IA_PrefixList;
                    while (currentNode->next != NULL)
                        currentNode = currentNode->next;
                    currentNode->next = newNode;
                }

                break;
            case DHCPv6_OPTION_STATUS_CODE:
                result.StatusCode = option;
                break;
        }

        int optPayloadLength = be16toh(readerPtr->OptionLength) + sizeof(uint16_t) + sizeof(uint16_t);
        unreadDataLength -= optPayloadLength;
        readerPtr += optPayloadLength;
    }

    result.success = true;

    return result;
}

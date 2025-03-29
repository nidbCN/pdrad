#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "dh_options.h"
#include "global.h"
#include <netinet/in.h>

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

dh_optPayload *dh_createOption_IA_PD(uint32_t id, const dh_optPayload prefixOptions[], size_t prefixOptionsLength) {
    dh_optPayload *payload = alloca(sizeof(dh_opt_IA_PD) + dh_optPayload_offset);
    dh_opt_IA_PD *thisOption = (dh_opt_IA_PD *) (payload + dh_optPayload_offset);

    thisOption->IA_id = htobe32(id);
    thisOption->Time1 = 0x00;
    thisOption->Time2 = 0x00;
    memcpy(thisOption->Options, prefixOptions, prefixOptionsLength);

    payload->OptionCode = DHCPv6_OPTION_IA_PD;
    payload->OptionLength = sizeof(dh_opt_IA_PD) + prefixOptionsLength;

    return payload;
}

dh_optPayload *
dh_createOption_IAPrefix(uint32_t preferredTime, uint32_t validTime, uint8_t prefixLength, struct in6_addr prefix) {
    uint optionLength = sizeof(dh_opt_IA_Prefix) - sizeof(uint8_t);

    dh_optPayload *payload = (dh_optPayload *) alloca(optionLength + dh_optPayload_offset);
    payload->OptionCode = DHCPv6_OPTION_IAPREFIX;
    payload->OptionLength = optionLength;

    dh_opt_IA_Prefix *option = (dh_opt_IA_Prefix *) (payload + dh_optPayload_offset);

    option->PreferredLifetime = htobe16(preferredTime);
    option->ValidLifetime = htobe16(validTime);
    option->PrefixLength = prefixLength;
    memcpy(&(option->Prefix), &prefix, sizeof(struct in6_addr));

    return payload;
}

dh_optPayload *dh_createOption_RapidCommit() {
    return dh_createOptPayload(DHCPv6_OPTION_RAPID_COMMIT, NULL, 0);
}

dh_optPayload *dh_createOption_ClientIdentifier_En(uint32_t enterpriseNumber, uint8_t id[], size_t idLength) {
    if (idLength <= 0)
        return NULL;

    dh_optPayload *payload = (dh_optPayload *) alloca(idLength + sizeof(uint16_t) + dh_optPayload_offset);
    payload->OptionCode = DHCPv6_OPTION_CLIENTID;
    payload->OptionLength = idLength + sizeof(uint16_t);

    dh_opt_ClientIdentifier *option = (dh_opt_ClientIdentifier *) (payload + dh_optPayload_offset);

    option->DUIdType = htobe16(DUId_En);
    option->Data.DUId_En.EnterpriseNumber = htobe32(enterpriseNumber);
    memcpy(option->Data.DUId_En.Identifier, id, idLength);

    return payload;
}

// unit: 1/100 sec
dh_optPayload *dh_createOption_ElapsedTime(uint16_t time) {
    dh_optPayload *payload = alloca(dh_optPayload_offset + sizeof(dh_opt_ElapsedTime));
    payload->OptionCode = DHCPv6_OPTION_ELAPSED_TIME;
    payload->OptionLength = sizeof(dh_opt_ElapsedTime);

    dh_opt_ElapsedTime *option = (dh_opt_ElapsedTime *) (payload + dh_optPayload_offset);
    option->ElapsedTime = htobe16(time);

    return payload;
}

dh_parsedOptions dh_parseOptions(const dh_pkt *pkt, size_t size) {
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
    if (pkt->MsgType != dh_ADVERTISE) {
        log_error("Wrong message type %d, expect dh_ADVERTISE.", pkt->MsgType);
        return result;
    }

    while (unreadDataLength > 0) {
        dh_optPayload *option = readerPtr;
        log_debug("Start read option at %p(offset %d bytes).", option, (void *) option - (void *) pkt);

        switch (be16toh(option->OptionCode)) {
            case DHCPv6_OPTION_CLIENTID:
                result.ClientIdentifier = option;
                break;
            case DHCPv6_OPTION_SERVERID:
                result.ServerIdentifier = option;
                break;
            case DHCPv6_OPTION_IA_PD:
                result.success = false;
                dh_optMultiPayload *newNode = new(dh_optMultiPayload);
                newNode->value = option;

                if (result.IA_PDList != NULL) {
                    result.IA_PDList = newNode;
                } else {
                    dh_optMultiPayload *currentNode = result.IA_PDList;
                    while (currentNode->next != NULL)
                        currentNode = currentNode->next;
                    currentNode->next = newNode;
                }
                break;
        }

        int optPayloadLength = be16toh(readerPtr->OptionLength) + sizeof(uint16_t) + sizeof(uint16_t);
        unreadDataLength -= optPayloadLength;
        readerPtr += optPayloadLength;
    }

    result.success = true;

    return result;
}

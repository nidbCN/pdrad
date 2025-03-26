#include <stdlib.h>
#include <string.h>
#include "DHCPv6_options.h"
#include "DHCPv6_header.h"
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#define new(T) ((T*)malloc(sizeof(T)))

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

    DHCPv6_opt_ElapsedTime *option = new(DHCPv6_opt_ElapsedTime);
    option->ElapsedTime = htobe16(time);
    bool result = createOptionPayload(payload, DHCPv6_OPTION_ELAPSED_TIME, option, sizeof(DHCPv6_opt_ElapsedTime));
    free(option);
    return result;
}

DHCPv6_optPayload *readerPtr = NULL;
size_t readerLength = 0;

bool DHCPv6_Reader_startRead(const DHCPv6_pkt *pkt, size_t size) {
    if (pkt->MsgType != ADVERTISE)
        return false;

    readerPtr = (void *) (pkt + 4);
    readerLength = size - 4;

    return true;
}

typedef struct _DHCPv6_Reader_result {
    bool readResult;
    DHCPv6_opt_IA_PD *IA_PD;
    DHCPv6_opt_ElapsedTime *ElapsedTIme;
    DHCPv6_opt_ClientIdentifier *ClientIdentifier;
    DHCPv6_optCollection_IA_Prefix_option *IA_PrefixList;
    DHCPv6_opt_StatusCode *StatusCode;
} DHCPv6_Reader_result;

DHCPv6_Reader_result DHCPv6_Reader_readOptions() {
    DHCPv6_Reader_result result = {
            .readResult = false
    };

    // invalid status
    if (readerLength <= 0 || readerPtr == NULL)
        return result;

    while (readerLength > 0) {
        int optionLength = be16toh(readerPtr->OptionLength);
        void *option = malloc(optionLength);
        memcpy(option, readerPtr->OptionData, optionLength);

        switch (be16toh(readerPtr->OptionCode)) {
            case DHCPv6_OPTION_CLIENTID:
                result.ClientIdentifier = option;
                break;
            case DHCPv6_OPTION_IA_PD:
                result.IA_PD = option;
                break;
            case DHCPv6_OPTION_IAPREFIX:
                result.readResult = false;
                DHCPv6_optCollection_IA_Prefix_option *node = new(DHCPv6_optCollection_IA_Prefix_option);
                node->value = option;

                if (result.IA_PrefixList != NULL) {
                    result.IA_PrefixList = node;
                } else {
                    DHCPv6_optCollection_IA_Prefix_option *pNode = result.IA_PrefixList;
                    while (pNode->next != NULL)
                        pNode = pNode->next;
                    pNode->next = node;
                }

                break;
            case DHCPv6_OPTION_STATUS_CODE:
                result.StatusCode = option;
                break;
        }
    }

    // clean
    readerLength = 0;
    readerPtr = NULL;

    result.readResult = true;
    return result;
}

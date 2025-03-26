#include <stddef.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "DHCPv6_reader.h"

DHCPv6_optPayload *readerPtr = NULL;
size_t readerLength = 0;

bool DHCPv6_Reader_startRead(const DHCPv6_pkt *pkt, size_t size) {
    if (pkt->MsgType != ADVERTISE)
        return false;

    readerPtr = (void *) (pkt + 4);
    readerLength = size - 4;

    return true;
}

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

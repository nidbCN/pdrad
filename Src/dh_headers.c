#include <stdlib.h>
#include <string.h>
#include "dh_header.h"

dh_pkt *dh_createPacket(enum dh_msgType msgType, dh_pkt_TransactionId transId,
                        dh_optPayload *optPtrList[], uint optPtrListLength) {
    size_t length = sizeof(uint8_t) + sizeof(dh_pkt_TransactionId);

    // count packet length
    for (int i = 0; i < optPtrListLength; ++i) {
        dh_optPayload *payload = optPtrList[i];
        length += payload->OptionLength + dh_optPayload_offset;
    }

    dh_pkt *packet = (dh_pkt *) alloca(length);
    packet->MsgType = msgType;
    packet->TransactionId = transId;

    // copy options
    size_t optionOffset = 0;
    for (int i = 0; i < optPtrListLength; ++i) {
        dh_optPayload *payload = optPtrList[i];
        memcpy(packet->Options, payload, payload->OptionLength + dh_optPayload_offset);
        optionOffset += payload->OptionLength + dh_optPayload_offset;
    }

    return packet;
}


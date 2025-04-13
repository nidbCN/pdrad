#include <stdlib.h>
#include <string.h>
#include "dh_header.h"

dh_pkt *dh_createPacket(enum dh_msgType msgType, dh_pkt_TransactionId transId,
                        (const dh_optPayload *)optPtrList[], uint optPtrListLength)
{
    size_t length = sizeof(uint8_t) + sizeof(dh_pkt_TransactionId);

    // count packet length
    for (int i = 0; i < optPtrListLength; ++i)
    {
        const dh_optPayload *payload = optPtrList[i];
        length += payload->OptionLength + dh_optPayload_offset;
    }

    dh_pkt *packet = (dh_pkt *)alloca(length);
    packet->MsgType = msgType;
    packet->TransactionId = transId;

    // copy options
    size_t optionOffset = 0;
    for (int i = 0; i < optPtrListLength; ++i)
    {
        const dh_optPayload *payload = optPtrList[i];
        memcpy(packet->Options, payload, payload->OptionLength + dh_optPayload_offset);
        optionOffset += payload->OptionLength + dh_optPayload_offset;
    }

    return packet;
}

dh_pkt *dh_createSolicitPacket(const dh_opt_ClientIdentifier *clientId, size_t clientSize, uint16_t elapsedTime, uint32_t IA_Id)
{
    return dh_createCustomedSolicitPacket(
        dh_createCustomOptPayload(DHCPv6_OPTION_CLIENTID, clientId, clientSize),
        dh_createOption_ElapsedTime(elapsedTime),
        dh_createOption_IA_PD(IA_Id, NULL, 0),
        0);
}

dh_pkt *dh_createRapidSolicitPacket(const dh_opt_ClientIdentifier *clientId, size_t clientSize, uint16_t elapsedTime, uint32_t IA_Id)
{
    dh_optPayload *rapidOption = dh_createOption_RapidCommit();

    return dh_createCustomedSolicitPacket(
        dh_createCustomOptPayload(DHCPv6_OPTION_CLIENTID, clientId, clientSize),
        dh_createOption_ElapsedTime(elapsedTime),
        dh_createOption_IA_PD(IA_Id, NULL, 0),
        1, rapid);
}

dh_pkt *dh_createCustomedSolicitPacket(
    const dh_optPayload *clientId,
    const dh_optPayload *elapsedTime,
    const dh_optPayload *IA_PD,
    uint optionsNum, ...)
{
    (const _dh_optPayload *)optionsPtrList[optionsNum + 3] = {(const _dh_optPayload *)0x00};

    if (clientId == NULL || elapsedTime == NULL || IA_PD == NULL)
    {
        // Invaild Arguments
        return NULL;
    }

    va_list vArg;
    va_start(vArg, optionsNum);

    // copy customed options
    for (uint i = 0; i < optionsNum; ++i)
    {
        optionsPtrList[i + 3] = va_arg(vArg, (const dh_optPayload *));
    }

    va_end(vArg);

    // generate random trans-id
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0)
    {
        return NULL;
    }

    dh_pkt_TransactionId transId = {};
    srandom(ts.tv_nsec ^ ts.tv_sec);

    if (sizeof(int) >= sizeof(dh_pkt_TransactionId))
    {
        int randomVal = random();
        memcpy(transId, &randomVal, sizeof(dh_pkt_TransactionId));
    }
    else
    {
        transId = {
            .TransactionId_0 = random(),
            .TransactionId_1 = random(),
            .TransactionId_2 = random(),
        };
    }

    optionsPtrList[0] = clientId;
    optionsPtrList[1] = elapsedTime;
    optionsPtrList[2] = IA_PD;

    return dh_createPacket(dh_SOLICIT, transId, optionsPtrList, sizeof(optionsPtrList));
}

dh_pkt *dh_createCustomedRequestPacket(const _dh_optPayload *clientId, uint16_t elapsedTime, uint32_t IA_Id)
{
    return NULL;
}

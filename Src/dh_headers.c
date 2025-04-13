#include <stdlib.h>
#include <string.h>
#include "dh_header.h"
#include "log.h"
#include "global.h"

size_t dh_createPacket(const dh_pkt **pktPtr, enum dh_msgType msgType, dh_pkt_TransactionId transId,
                       const dh_optPayload *optPtrList[], uint optPtrListLength) {
    size_t length = sizeof(uint8_t) + sizeof(dh_pkt_TransactionId);

    // count packet length
    for (int i = 0; i < optPtrListLength; ++i) {
        const dh_optPayload *payload = optPtrList[i];
        length += payload->OptionLength + dh_optPayload_offset;
    }

    dh_pkt *packet = (dh_pkt *) malloc(length);
    packet->MsgType = msgType;
    packet->TransactionId = transId;

    // copy options
    size_t optionOffset = 0;
    for (int i = 0; i < optPtrListLength; ++i) {
        const dh_optPayload *payload = optPtrList[i];
        dh_optPayload *buffer = (dh_optPayload *) ((size_t) (packet->Options) + optionOffset);

        memcpy((void *) buffer, payload, payload->OptionLength + dh_optPayload_offset);
        optionOffset += payload->OptionLength + dh_optPayload_offset;

        buffer->OptionCode = htobe16(buffer->OptionCode);
        buffer->OptionLength = htobe16(buffer->OptionLength);
    }
    *pktPtr = packet;
    return length;
}

size_t dh_createSolicitPacket(const dh_pkt **pktPtr, const dh_opt_ClientIdentifier *clientId, size_t clientSize,
                              uint16_t elapsedTime,
                              uint32_t IA_Id) {
    return dh_CreateCustomizedSolicitPacket(
            pktPtr,
            dh_createCustomOptPayload(DHCPv6_OPTION_CLIENTID, (void *) clientId, clientSize),
            dh_createOption_ElapsedTime(elapsedTime),
            dh_createOption_IA_PD(IA_Id, NULL, 0),
            0);
}

size_t dh_createRapidSolicitPacket(const dh_pkt **pktPtr, const dh_opt_ClientIdentifier *clientId, size_t clientSize,
                                   uint16_t elapsedTime,
                                   uint32_t IA_Id) {
    dh_optPayload *rapidOption = dh_createOption_RapidCommit();

    return dh_CreateCustomizedSolicitPacket(
            pktPtr,
            dh_createCustomOptPayload(DHCPv6_OPTION_CLIENTID, clientId, clientSize),
            dh_createOption_ElapsedTime(elapsedTime),
            dh_createOption_IA_PD(IA_Id, NULL, 0),
            1, rapidOption);
}

size_t dh_CreateCustomizedSolicitPacket(
        const dh_pkt **pktPtr,
        const dh_optPayload *clientId,
        const dh_optPayload *elapsedTime,
        const dh_optPayload *IA_PD,
        uint optionsNum, ...) {
    const dh_optPayload **optionsPtrList = malloc(sizeof(dh_optPayload *) * (optionsNum + 3));

    if (clientId == NULL || elapsedTime == NULL || IA_PD == NULL) {
        // Invalid Arguments
        return -1;
    }

    va_list vArg;
    va_start(vArg, optionsNum);

    // copy customize options
    for (uint i = 0; i < optionsNum; ++i) {
        optionsPtrList[i + 3] = va_arg(vArg, const dh_optPayload *);
    }

    va_end(vArg);

    // generate random trans-id
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        return -1;
    }

    dh_pkt_TransactionId transId = {};
    srandom(ts.tv_nsec ^ ts.tv_sec);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    if (sizeof(int) >= sizeof(dh_pkt_TransactionId)) {
        long randomVal = random();
        memcpy(&transId, &randomVal, sizeof(dh_pkt_TransactionId));
    } else {
        transId.TransactionId_0 = random();
        transId.TransactionId_1 = random();
        transId.TransactionId_2 = random();
    }
#pragma clang diagnostic pop

    optionsPtrList[0] = clientId;
    optionsPtrList[1] = elapsedTime;
    optionsPtrList[2] = IA_PD;

    return dh_createPacket(pktPtr, dh_SOLICIT, transId, optionsPtrList, optionsNum + 3);
}

size_t dh_createCustomizedRequestPacket(const dh_pkt **pktPtr, const dh_optPayload *clientId, uint16_t elapsedTime,
                                        uint32_t IA_Id) {
    return -1;
}

dh_parsedOptions dh_parseOptions(const dh_pkt *pkt, size_t size) {
    log_debug("Start parse option, address: %p, length: %d.", pkt, size);

    dh_optPayload *readerPtr = (dh_optPayload *) ((size_t) pkt + dh_pkt_offset);
    size_t unreadDataLength = size - dh_pkt_offset;

    log_trace("Skip header, address: %p, length: %d.", readerPtr, unreadDataLength);

    dh_parsedOptions result = {.success = false};

    // invalid status
    if (unreadDataLength <= 0 || readerPtr == NULL) {
        log_error("Invalid packet with content length %d and address %p.", unreadDataLength, readerPtr);
        return result;
    }

    // invalid type
    if (pkt->MsgType != dh_ADVERTISE && pkt->MsgType != dh_REPLY) {
        log_error("Wrong message type %d, expect dh_ADVERTISE or dh_REPLY.", pkt->MsgType);
        return result;
    }

    while (true) {
        dh_optPayload *option = readerPtr;
        log_debug("Option parsing: ptr=%p,offset=%td bytes,remain=%td bytes.",
                  option, (uintptr_t) option - (uintptr_t) pkt, unreadDataLength);

        option->OptionCode = be16toh(option->OptionCode);
        option->OptionLength = be16toh(option->OptionLength);

        const size_t optPayloadLength = option->OptionLength + dh_optPayload_offset;

        log_debug("Option header: code=%d,length=%d bytes.", option->OptionCode, option->OptionLength);

        switch (option->OptionCode) {
            case DHCPv6_OPTION_CLIENTID:
                result.ClientIdentifier = option;
                break;

            case DHCPv6_OPTION_SERVERID:
                result.ServerIdentifier = option;
                break;

            case DHCPv6_OPTION_IA_PD: {
                result.success = false;
                dh_optMultiPayload *newNode = new(dh_optMultiPayload);  // 更安全的分配方式
                if (newNode == NULL) {
                    log_error("Memory allocation failed");
                }

                newNode->value = option;
                newNode->next = NULL;

                if (result.IA_PDList == NULL) {
                    result.IA_PDList = newNode;
                } else {
                    // 使用尾指针优化链表插入
                    dh_optMultiPayload **current = &result.IA_PDList;
                    while (*current != NULL) {
                        current = &(*current)->next;
                    }
                    *current = newNode;
                }
                break;
            }

            default:
                // 处理未知选项或跳过
                break;
        }

        unreadDataLength -= optPayloadLength;
        if (unreadDataLength <= 0) {
            break;
        }

        // 使用指针算术确保对齐
        readerPtr = (dh_optPayload *) ((uint8_t *) readerPtr + optPayloadLength);

        // 检查指针是否越界
        if ((uintptr_t) readerPtr > (uintptr_t) pkt + size) {
            log_error("Option parsing out of bounds");
            break;
        }
    }

    result.success = true;

    return result;
}
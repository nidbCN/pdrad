#include <stdlib.h>
#include <string.h>
#include "ndp_packets.h"
#include "ndp_options.h"

ndp_ra *ndp_ra_createPacket(
        uint8_t curHopLimit,
        uint16_t routerLifeTime,
        uint32_t reachableTime,
        uint32_t reTransTimer,
        ndp_optPayload *optionsList[],
        uint8_t optionsNum
) {

    size_t length = sizeof(ndp_ra);

    for (int i = 0; i < optionsNum; ++i) {
        length += optionsList[i]->Length * 8;
    }

    ndp_ra *pkt = (ndp_ra *) malloc(length);
    pkt->Type = 134;
    pkt->Code = 0;
    pkt->CheckSum = 0x00; // TODO: impl check sum
    pkt->CurHopLimit = curHopLimit;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    pkt->Flags = (!M) | (!O) & (0xC0);
#pragma clang diagnostic pop
    pkt->RouterLifetime = routerLifeTime;
    pkt->ReachableTime = reachableTime;
    pkt->ReTransTimer = reTransTimer;

    ndp_optPayload *options = (ndp_optPayload *) pkt->Options;
    for (int i = 0; i < optionsNum; ++i) {
        uint optionLength = optionsList[i]->Length * 8;
        memcpy(options, optionsList[i], optionLength);
        options = (ndp_optPayload *) ((uintptr_t) options + optionLength);
    }

    return pkt;
}
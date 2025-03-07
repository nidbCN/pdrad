#include <stdbool.h>
#include "DHCPv6_header.h"

// 判断消息类型是否是 Client/Server 消息格式 (msg-type, id, Options)
bool DHCPv6_isCSMessage(uint16_t msg_type) {
    switch (msg_type) {
        case SOLICIT:
        case ADVERTISE:
        case REQUEST:
        case CONFIRM:
        case RENEW:
        case REBIND:
        case REPLY:
        case RELEASE:
        case DECLINE:
        case RECONFIGURE:
        case INFORMATION_REQUEST:
            return true;  // 属于 Client/Server 格式的消息
        case RELAY_FORW:
        case RELAY_REPL:
        default:
            return false; // 未知类型，默认认为不是
    }
}

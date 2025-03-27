#ifndef PDRAD_DHCPv6_OPTIONS_H
#define PDRAD_DHCPv6_OPTIONS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "DHCPv6_header.h"

enum dh_options {
    DHCPv6_OPTION_CLIENTID = 1,
    DHCPv6_OPTION_SERVERID = 2,
    DHCPv6_OPTION_IA_NA = 3,
    DHCPv6_OPTION_IA_TA = 4,
    DHCPv6_OPTION_IAADDR = 5,
    DHCPv6_OPTION_ORO = 6,
    DHCPv6_OPTION_PREFERENCE = 7,
    DHCPv6_OPTION_ELAPSED_TIME = 8,
    DHCPv6_OPTION_RELAY_MSG = 9,
    DHCPv6_OPTION_AUTH = 11,
    DHCPv6_OPTION_UNICAST = 12,
    DHCPv6_OPTION_STATUS_CODE = 13,
    DHCPv6_OPTION_RAPID_COMMIT = 14,
    DHCPv6_OPTION_USER_CLASS = 15,
    DHCPv6_OPTION_VENDOR_CLASS = 16,
    DHCPv6_OPTION_VENDOR_OPTS = 17,
    DHCPv6_OPTION_INTERFACE_ID = 18,
    DHCPv6_OPTION_RECONF_MSG = 19,
    DHCPv6_OPTION_RECONF_ACCEPT = 20,
    DHCPv6_OPTION_SIP_SERVER_D = 21,
    DHCPv6_OPTION_SIP_SERVER_A = 22,
    DHCPv6_OPTION_DNS_SERVERS = 23,
    DHCPv6_OPTION_DOMAIN_LIST = 24,
    DHCPv6_OPTION_IA_PD = 25,
    DHCPv6_OPTION_IAPREFIX = 26,
    DHCPv6_OPTION_NIS_SERVERS = 27,
    DHCPv6_OPTION_NISP_SERVERS = 28,
    DHCPv6_OPTION_NIS_DOMAIN_NAME = 29,
    DHCPv6_OPTION_NISP_DOMAIN_NAME = 30,
    DHCPv6_OPTION_SNTP_SERVERS = 31,
    DHCPv6_OPTION_INFORMATION_REFRESH_TIME = 32,
    DHCPv6_OPTION_BCMCS_SERVER_D = 33,
    DHCPv6_OPTION_BCMCS_SERVER_A = 34,
    DHCPv6_OPTION_GEOCONF_CIVIC = 36,
    DHCPv6_OPTION_REMOTE_ID = 37,
    DHCPv6_OPTION_SUBSCRIBER_ID = 38,
    DHCPv6_OPTION_CLIENT_FQDN = 39,
    DHCPv6_OPTION_PANA_AGENT = 40,
    DHCPv6_OPTION_NEW_POSIX_TIMEZONE = 41,
    DHCPv6_OPTION_NEW_TZDB_TIMEZONE = 42,
    DHCPv6_OPTION_ERO = 43,
    DHCPv6_OPTION_LQ_QUERY = 44,
    DHCPv6_OPTION_CLIENT_DATA = 45,
    DHCPv6_OPTION_CLT_TIME = 46,
    DHCPv6_OPTION_LQ_RELAY_DATA = 47,
    DHCPv6_OPTION_LQ_CLIENT_LINK = 48,
    DHCPv6_OPTION_MIP6_HNIDF = 49,
    DHCPv6_OPTION_MIP6_VDINF = 50,
    DHCPv6_OPTION_V6_LOST = 51,
    DHCPv6_OPTION_CAPWAP_AC_V6 = 52,
    DHCPv6_OPTION_RELAY_ID = 53,
    DHCPv6_OPTION_IPV6_Address_MoS = 54,
    DHCPv6_OPTION_IPV6_FQDN_MoS = 55,
    DHCPv6_OPTION_NTP_SERVER = 56,
    DHCPv6_OPTION_V6_ACCESS_DOMAIN = 57,
    DHCPv6_OPTION_SIP_UA_CS_LIST = 58,
    DHCPv6_OPTION_OPT_BOOTFILE_URL = 59,
    DHCPv6_OPTION_OPT_BOOTFILE_PARAM = 60,
    DHCPv6_OPTION_CLIENT_ARCH_TYPE = 61,
    DHCPv6_OPTION_NII = 62,
    DHCPv6_OPTION_GEOLOCATION = 63,
    DHCPv6_OPTION_AFTR_NAME = 64,
    DHCPv6_OPTION_ERP_LOCAL_DOMAIN_NAME = 65,
    DHCPv6_OPTION_RSOO = 66,
    DHCPv6_OPTION_PD_EXCLUDE = 67,
    DHCPv6_OPTION_VSS = 68,
    DHCPv6_OPTION_MIP6_IDINF = 69,
    DHCPv6_OPTION_MIP6_UDINF = 70,
    DHCPv6_OPTION_MIP6_HNP = 71,
    DHCPv6_OPTION_MIP6_HAA = 72,
    DHCPv6_OPTION_MIP6_HAF = 73,
    DHCPv6_OPTION_RDNSS_SELECTION = 74,
    DHCPv6_OPTION_KRB_PRINCIPAL_NAME = 75,
    DHCPv6_OPTION_KRB_REALM_NAME = 76,
    DHCPv6_OPTION_KRB_DEFAULT_REALM_NAME = 77,
    DHCPv6_OPTION_KRB_KDC = 78,
    DHCPv6_OPTION_CLIENT_LINKLAYER_ADDR = 79,
    DHCPv6_OPTION_LINK_ADDRESS = 80,
    DHCPv6_OPTION_RADIUS = 81,
    DHCPv6_OPTION_SOL_MAX_RT = 82,
    DHCPv6_OPTION_INF_MAX_RT = 83,
    DHCPv6_OPTION_ADDRSEL = 84,
    DHCPv6_OPTION_ADDRSEL_TABLE = 85,
    DHCPv6_OPTION_V6_PCP_SERVER = 86,
    DHCPv6_OPTION_DHCPV4_MSG = 87,
    DHCPv6_OPTION_DHCP4_O_DHCP6_SERVER = 88,
    DHCPv6_OPTION_S46_RULE = 89,
    DHCPv6_OPTION_S46_BR = 90,
    DHCPv6_OPTION_S46_DMR = 91,
    DHCPv6_OPTION_S46_V4V6BIND = 92,
    DHCPv6_OPTION_S46_PORTPARAMS = 93,
    DHCPv6_OPTION_S46_CONT_MAPE = 94,
    DHCPv6_OPTION_S46_CONT_MAPT = 95,
    DHCPv6_OPTION_S46_CONT_LW = 96,
    DHCPv6_OPTION_4RD = 97,
    DHCPv6_OPTION_4RD_MAP_RULE = 98,
    DHCPv6_OPTION_4RD_NON_MAP_RULE = 99,
    DHCPv6_OPTION_LQ_BASE_TIME = 100,
    DHCPv6_OPTION_LQ_START_TIME = 101,
    DHCPv6_OPTION_LQ_END_TIME = 102,
    DHCPv6_OPTION_DHCP_Captive_Portal = 103,
    DHCPv6_OPTION_MPL_PARAMETERS = 104,
    DHCPv6_OPTION_ANI_ATT = 105,
    DHCPv6_OPTION_ANI_NETWORK_NAME = 106,
    DHCPv6_OPTION_ANI_AP_NAME = 107,
    DHCPv6_OPTION_ANI_AP_BSSID = 108,
    DHCPv6_OPTION_ANI_OPERATOR_ID = 109,
    DHCPv6_OPTION_ANI_OPERATOR_REALM = 110,
    DHCPv6_OPTION_S46_PRIORITY = 111,
    DHCPv6_OPTION_MUD_URL_V6 = 112,
    DHCPv6_OPTION_V6_PREFIX64 = 113,
    DHCPv6_OPTION_F_BINDING_STATUS = 114,
    DHCPv6_OPTION_F_CONNECT_FLAGS = 115,
    DHCPv6_OPTION_F_DNS_REMOVAL_INFO = 116,
    DHCPv6_OPTION_F_DNS_HOST_NAME = 117,
    DHCPv6_OPTION_F_DNS_ZONE_NAME = 118,
    DHCPv6_OPTION_F_DNS_FLAGS = 119,
    DHCPv6_OPTION_F_EXPIRATION_TIME = 120,
    DHCPv6_OPTION_F_MAX_UNACKED_BNDUPD = 121,
    DHCPv6_OPTION_F_MCLT = 122,
    DHCPv6_OPTION_F_PARTNER_LIFETIME = 123,
    DHCPv6_OPTION_F_PARTNER_LIFETIME_SENT = 124,
    DHCPv6_OPTION_F_PARTNER_DOWN_TIME = 125,
    DHCPv6_OPTION_F_PARTNER_RAW_CLT_TIME = 126,
    DHCPv6_OPTION_F_PROTOCOL_VERSION = 127,
    DHCPv6_OPTION_F_KEEPALIVE_TIME = 128,
    DHCPv6_OPTION_F_RECONFIGURE_DATA = 129,
    DHCPv6_OPTION_F_RELATIONSHIP_NAME = 130,
    DHCPv6_OPTION_F_SERVER_FLAGS = 131,
    DHCPv6_OPTION_F_SERVER_STATE = 132,
    DHCPv6_OPTION_F_START_TIME_OF_STATE = 133,
    DHCPv6_OPTION_F_STATE_EXPIRATION_TIME = 134,
    DHCPv6_OPTION_RELAY_PORT = 135,
    DHCPv6_OPTION_IPV6_Address_ANDSF = 136
};

typedef struct _dh_opt_status_code {
    uint16_t StatusCode;
    char StatusMessage[];
} __attribute__((packed)) dh_opt_StatusCode;

typedef struct _dh_opt_IA_PD {
    uint32_t IA_id;
    uint32_t Time1;
    uint32_t Time2;
} __attribute__((packed)) dh_opt_IA_PD;

typedef struct _dh_opt_IA_Prefix_option {
    uint32_t PreferredLifetime;
    uint32_t ValidLifetime;
    uint8_t PrefixLength;
    uint8_t IPv6_Prefix[16];
} __attribute__((packed)) dh_opt_IA_Prefix;

typedef struct _dh_opt_RapidCommit {
} dh_opt_RapidCommit;

typedef struct _dh_opt_ElapsedTime {
    uint16_t ElapsedTime;
} __attribute__((packed)) dh_opt_ElapsedTime;

enum dh_DUIdType {
    DUId_LLT = 1,
    DUId_En = 2,
    DUId_LL = 3,
    DUId_UUId = 4,
};

typedef struct _dh_opt_ClientIdentifier {
    uint16_t DUIdType;
    union {
        struct {
            uint16_t HardwareType;
            uint32_t Time;
            uint8_t LinkLayerAddress[];
        } __attribute__((packed)) DUId_LLT;
        struct {
            uint32_t EnterpriseNumber;
            uint8_t Identifier[];
        } __attribute__((packed)) DUId_En;
        struct {
            uint16_t HardwareType;
            uint8_t LinkLayerAddress[];
        } __attribute__((packed)) DUId_LL;
        uint32_t DUId_UUId[4];
    } __attribute__((packed)) Data;
} __attribute__((packed)) dh_opt_ClientIdentifier;

typedef struct _dh_optPayload {
    uint16_t OptionCode;
    uint16_t OptionLength;
    uint8_t OptionData[];
} __attribute__((packed)) dh_optPayload;

typedef struct _dh_optMultiPayload {
    dh_optPayload *value;
    struct _dh_optMultiPayload *next;
} dh_optMultiPayload;

typedef struct _DHCPv6_Reader_result {
    bool success;
    dh_optPayload *IA_PD;
    dh_optPayload *ElapsedTIme;
    dh_optPayload *ClientIdentifier;
    dh_optMultiPayload *IA_PrefixList;
    dh_optPayload *StatusCode;
} dh_parsedOptions;

#define dh_optPayload_offset (sizeof(uint16_t) + sizeof(uint16_t))

dh_optPayload *dh_createCustomOptPayload(uint16_t optionCode, void *optionData, size_t optionDataLength);

dh_optPayload *dh_createOptPayload(enum dh_options optionCode, void *optionData, size_t optionDataLength);

dh_optPayload *dh_createOption_IA_PD(uint32_t id, uint32_t preferredRenewalTimeSec, uint32_t preferredRebindTime);

dh_optPayload *dh_createOption_RapidCommit();

bool dh_createOption_ElapsedTime(dh_optPayload *payload, uint16_t time);

bool
dh_createOption_ClientIdentifier_En(dh_optPayload *payload, uint32_t enterpriseNumber, uint8_t id[], size_t idLength);

#endif //PDRAD_DHCPv6_OPTIONS_H

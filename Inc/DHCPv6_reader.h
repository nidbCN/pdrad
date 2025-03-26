#ifndef PDRAD_DHCPV6_READER_H
#define PDRAD_DHCPV6_READER_H

#include <stdbool.h>
#include "DHCPv6_options.h"

typedef struct _DHCPv6_Reader_result {
    bool readResult;
    DHCPv6_opt_IA_PD *IA_PD;
    DHCPv6_opt_ElapsedTime *ElapsedTIme;
    DHCPv6_opt_ClientIdentifier *ClientIdentifier;
    DHCPv6_optCollection_IA_Prefix_option *IA_PrefixList;
    DHCPv6_opt_StatusCode *StatusCode;
} DHCPv6_Reader_result;

#endif //PDRAD_DHCPV6_READER_H

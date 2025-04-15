typedef struct _ndp_ra
{
    uint8_t Type;
    uint8_t Code;
    uint16_t CheckSum;
    uint8_t CurHopLimit;
    uint8_t Flags;
    uint16_t RouterLifetime;
    uint32_t ReachableTime;
    uint32_t RetransTimer;
    uint8_t Options[];
} ndp_ra;

enum ndp_ra_flag
{
    M = 0x80,
    O = 0x40,
    PrfHigh = 0x08,
    PrfMedium = 0x00,
    PrfLow = 0x18,
    PrfInvaild = 0x10,
    Reserved = 0x00
};

ndp_ra *ndp_createTypicalPacket(
    ndp_optPayload *prefix,
    ndp_optPayload *mtu,
    ndp_optPayload *sourceLinkAddr)
{
    if(prefix == NULL || mtu == NULL || sourceLinkAddr == NULL)
    return NULL;

    ndp_ra* pkt = malloc();

    
}
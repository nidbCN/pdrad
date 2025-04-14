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
    RESERVERD = 0x00
};
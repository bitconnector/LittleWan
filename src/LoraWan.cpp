#include "LoraWan.hpp"

LoraWan::LoraWan(RadioConfig &_radio_cfg) {}

LoraWan::~LoraWan() {}

void LoraWan::setNwkSKey(const char *NwkKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        NwkSKey[i] = ASCII2Hex(&NwkKey_in[i * 2]);
}

void LoraWan::setAppSKey(const char *ApskKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        AppSKey[i] = ASCII2Hex(&ApskKey_in[i * 2]);
}

void LoraWan::setDevAddr(const char *devAddr_in)
{
    for (uint8_t i = 0; i < 4; ++i)
        DevAddr[i] = ASCII2Hex(&devAddr_in[i * 2]);
}

void LoraWan::setFrameCounter(uint16_t up, uint16_t down)
{
    frameCounterUp = up;
    frameCounterDown = down;
}

void LoraWan::calculateMIC(char *buf, int size, char *key)
{
}

unsigned char LoraWan::ASCII2Hex(const char str[2])
{
    unsigned char ASCII = 0;
    // High Nibble
    if (str[0] >= 'A' && str[0] <= 'F')
        ASCII = str[0] - 'A' + 10;
    else if (str[0] >= 'a' && str[0] <= 'f')
        ASCII = str[0] - 'a' + 10;
    else
        ASCII = str[0] - '0';
    ASCII <<= 4;
    // Low Nibble
    if (str[1] >= 'A' && str[1] <= 'F')
        ASCII = str[1] - 'A' + 10;
    else if (str[1] >= 'a' && str[1] <= 'f')
        ASCII = str[1] - 'a' + 10;
    else
        ASCII = str[1] - '0';
    return ASCII;
}

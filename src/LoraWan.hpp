/* Packing a LoraWan message taking care of encryption, MIC and headers
*/

#pragma once

#include "RadioConfig.hpp"
#include "AES-128.h"

class LoraWan
{
public:
    LoraWan();
    LoraWan(RadioConfig &_radio_cfg);
    ~LoraWan();

    void setNwkSKey(const char *NwkKey_in);
    void setAppSKey(const char *ApskKey_in);
    void setDevAddr(const char *devAddr_in);
    void setFrameCounter(uint16_t up, uint16_t down);

    void sendUplink(char *data, uint8_t len, bool confirm, uint8_t port);

    int setMHDRandFHDR(char *buf);
    int setFRMPayload(char *buf, int offset, char *data, uint8_t port);

    void encryptPayload(char *buf, char *key, char *data);
    int calculateMIC(char *buf, uint8_t size, bool direction);

protected:
    unsigned char DevAddr[4];
    unsigned char NwkSKey[16];
    unsigned char AppSKey[16];
    uint16_t frameCounterUp, frameCounterDown;

    RadioConfig radio_config;

private:
    unsigned char ASCII2Hex(const char str[2]);
    void Generate_Keys(unsigned char *Key, unsigned char *K1, unsigned char *K2);
    void Shift_Left(unsigned char *Data);
};

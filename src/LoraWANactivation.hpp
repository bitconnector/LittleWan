#pragma once

#include "LoraWANmessage.hpp"

class LoraWANactivation
{
public:
    unsigned char DevEUI[8];
    unsigned char JoinEUI[8];
    unsigned char AppKey[16];
    uint16_t DevNonce = 0;
    LoraWANmessage *MSG = nullptr;

public:
    LoraWANactivation();
    LoraWANactivation(LoraWANmessage *message);
    ~LoraWANactivation();

    void setDevEUI(const char *devEUI_in);
    void setJoinEUI(const char *joinEUI_in);
    void setAppKey(const char *appKey_in);
    void setOTAA(const char *devEUI_in, const char *joinEUI_in, const char *appKey_in);

    void joinmsg();

    void setDevNonce(uint16_t nonce);
};

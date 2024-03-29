#pragma once

#include "AES-128.h"
#include "inttypes.h"

#define MAX_DATA_SIZE 235

class LoraWANmessage
{
public:
    unsigned char NwkSKey[16];
    unsigned char AppSKey[16];
    unsigned char DevAddr[4];
    uint16_t frameCounterUp;
    uint16_t frameCounterDown;
    unsigned char *data;
    unsigned char *payload;
    unsigned char dataLen = 0;

public:
    LoraWANmessage();
    LoraWANmessage(unsigned char *data);
    ~LoraWANmessage();

    void setDevAddr(const char *devAddr_in);
    void setNwkSKey(const char *NwkKey_in);
    void setAppSKey(const char *ApskKey_in);
    void setAPB(const char *devAddr_in, const char *NwkKey_in, const char *ApskKey_in);

    void uplink(char *data, uint8_t len, uint8_t port, bool confirm = 0);
    unsigned char *getPayload() { return data; }
    unsigned char getPayloadSize() { return dataLen; }

    void setFrameCounterUp(uint16_t up);
    void setFrameCounterDown(uint16_t down);

    void setMHDR(bool confirm = 0);
    void setFHDR(unsigned char FCtrl = 0, char *FOpts = nullptr);
    void setFRMPayload(uint8_t port, char *data = nullptr, uint8_t len = 0);
    void setMIC();

    bool checkHDR(char *data, uint8_t len);
    char getFtype(char data);
    unsigned char getFoptsLen(char data);

    void calculateMIC(char *data, uint8_t len, uint16_t counter, unsigned char *mic, bool direction = 0); //default Down
    void Encrypt_Payload(unsigned char *Buffer, unsigned char buffer_size, unsigned char *Key, uint16_t counter, bool direction = 0);
    unsigned char ASCII2Hex(const char str[2]);
    void Generate_Keys(unsigned char *Key, unsigned char *K1, unsigned char *K2);
    void Shift_Left(unsigned char *Data);
};

#ifdef LITTLEWANDEBUGPORT
#include <Arduino.h>
void printPackage(char *data, uint16_t size, bool structure);
#endif

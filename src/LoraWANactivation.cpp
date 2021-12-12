#include "LoraWANactivation.hpp"

LoraWANactivation::LoraWANactivation() {}
LoraWANactivation::LoraWANactivation(LoraWANmessage *message)
{
    MSG = message;
}
LoraWANactivation::~LoraWANactivation() {}

void LoraWANactivation::setDevEUI(const char *devEUI_in)
{
    for (uint8_t i = 0; i < 8; ++i)
        DevEUI[i] = MSG->ASCII2Hex(&devEUI_in[(7 - i) * 2]);
}

void LoraWANactivation::setJoinEUI(const char *joinEUI_in)
{
    for (uint8_t i = 0; i < 8; ++i)
        JoinEUI[i] = MSG->ASCII2Hex(&joinEUI_in[(7 - i) * 2]);
}

void LoraWANactivation::setAppKey(const char *appKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        AppKey[i] = MSG->ASCII2Hex(&appKey_in[(15 - i) * 2]);
}

void LoraWANactivation::setOTAA(const char *devEUI_in, const char *joinEUI_in, const char *appKey_in)
{
    setDevEUI(devEUI_in);
    setJoinEUI(joinEUI_in);
    setAppKey(appKey_in);
}

void LoraWANactivation::setDevNonce(uint16_t nonce)
{
    DevNonce = nonce;
}

void LoraWANactivation::joinmsg()
{
    MSG->data[0] = 0x00;           //Join request
    for (int i = 0; i < 8; i++) //load JoinEUI
        MSG->data[i + 1] = JoinEUI[i];
    for (int i = 0; i < 8; i++) //load DevEUI
        MSG->data[i + 9] = DevEUI[i];
    MSG->data[17] = DevNonce;
    MSG->data[18] = DevNonce >> 8;

    //calculating MIC
    unsigned char MIC_Data[16];
    for (uint8_t i = 0; i < 16; i++)
        MIC_Data[i] = MSG->data[i];

    AES_Encrypt(MIC_Data, AppKey);

    MIC_Data[0] ^= MSG->data[16];
    MIC_Data[1] ^= MSG->data[17];
    MIC_Data[2] ^= MSG->data[18];
    MIC_Data[3] ^= 0x80;

    unsigned char Key_K1[16] = {0};
    unsigned char Key_K2[16] = {0};

    MSG->Generate_Keys(AppKey, Key_K1, Key_K2);
    for (int i = 0; i < 16; i++)
        MIC_Data[i] ^= Key_K2[i];

    AES_Encrypt(MIC_Data, AppKey);

    for (int8_t i = 0; i < 4; i++)
        MSG->data[19 + i] = MIC_Data[i];
    MSG->dataLen = 23;
    DevNonce++;
}

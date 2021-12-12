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
        AppKey[i] = MSG->ASCII2Hex(&appKey_in[i * 2]);
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
    DevNonce++;
    MSG->data[0] = 0x00;        //Join request
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
}

bool LoraWANactivation::checkJoin(char *data, uint8_t len)
{
    if (len < 13) //too small for a LoraWan join accept
        return false;

    char mhdr = MSG->getFtype(data[0]);
    if (mhdr != 0x20) //join accept
        return false; //MHDR.FType doesnt match join accept

    //Decrypt the data
    for (int i = 0x00; i < ((MSG->dataLen - 1) / 16); i++)
        AES_Encrypt(&(MSG->data[(i * 16) + 1]), AppKey);

    unsigned char MIC_Data[16] = {0};
    unsigned char Key_K1[16] = {0};
    unsigned char Key_K2[16] = {0};

    unsigned char datalen = MSG->dataLen - 4;

    unsigned char Number_of_Blocks = datalen / 16;
    unsigned char Incomplete_Block_Size = datalen % 16;
    if (Incomplete_Block_Size != 0)
        Number_of_Blocks++;

    MSG->Generate_Keys(AppKey, Key_K1, Key_K2);
    unsigned char *key = Key_K2;

    for (uint8_t j = 0; j < Number_of_Blocks; j++)
    {
        for (uint8_t i = 0; i < 16; i++) //Copy new data and XOR with old
        {
            uint8_t dataptr = (j * 16) + i;
            if (dataptr == datalen)
            {
                MIC_Data[i] ^= 0x80;
                break;
            }
            MIC_Data[i] ^= data[dataptr];
        }

        if (j == (Number_of_Blocks - 1))
        {
            if ((len % 16) == 0)
                key = Key_K1;
            for (int i = 0; i < 16; i++)
                MIC_Data[i] ^= key[i];
        }
        AES_Encrypt(MIC_Data, AppKey);
    }

    unsigned char *micData = (unsigned char *)&MSG->data[len - 4];
    for (int i = 0; i < 4; i++)
        if (micData[i] != MIC_Data[i])
            return false; //MIC is wrong

    uint8_t i;

    for (i = 0; i < 4; i++)
        MSG->DevAddr[i] = MSG->data[i + 7];

    //Calculate Network Session Key
    MSG->NwkSKey[0] = 0x01;
    for (i = 0; i < 3; i++) //Load AppNonce
        MSG->NwkSKey[i + 1] = MSG->data[i + 1];
    for (i = 0; i < 3; i++) //Load NetID
        MSG->NwkSKey[i + 4] = MSG->data[i + 4];

    //Load Dev Nonce
    MSG->NwkSKey[7] = DevNonce;
    MSG->NwkSKey[8] = DevNonce >> 8;

    //Pad with zeros
    for (i = 9; i <= 15; i++)
        MSG->NwkSKey[i] = 0x00;

    //Copy to AppSkey
    for (i = 0x00; i < 16; i++)
        MSG->AppSKey[i] = MSG->NwkSKey[i];

    //Change first byte of AppSKey
    MSG->AppSKey[0] = 0x02;

    //Calculate the keys
    AES_Encrypt(MSG->NwkSKey, AppKey);
    AES_Encrypt(MSG->AppSKey, AppKey);

    MSG->frameCounterUp = 0;
    MSG->frameCounterDown = 0;

    return true;
}

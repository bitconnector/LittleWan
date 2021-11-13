#include "LoraWan.hpp"
#include "RadioConfig.hpp"

LoraWan::~LoraWan() {}

LoraWan::LoraWan() {}

LoraWan::LoraWan(RadioConfig &_radio_cfg)
{
    radio_config = _radio_cfg;
}

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

void LoraWan::sendUplink(char *data, uint8_t len, bool confirm, uint8_t port)
{
    char buf[200] = {0};
    int size = setMHDRandFHDR(buf);
    if (len > 0)
    {
        buf[size++] = (char)port;
        //TODO: encrypt data and copy to buf
    }
    size = calculateMIC(buf, size, 1, NwkSKey);

    //radio_config.sendMessage();
#ifdef DEBUG
    Serial.print("+ Uplink:");
    for (int i = 0; i < size; i++)
        Serial.printf(" %02x", buf[i]);
    Serial.println("");
#endif
}

int LoraWan::setMHDRandFHDR(char *buf)
{
    buf[0] = 0x40;       //[0] MHDR -> unconfirmed uplink
    buf[1] = DevAddr[3]; //[1-4] FHDR.DevAddr
    buf[2] = DevAddr[2];
    buf[3] = DevAddr[1];
    buf[4] = DevAddr[0];
    buf[5] = 0x00;                      //[5] FHDR.FCtrl (frame control)
    buf[6] = frameCounterDown & 0x00FF; //[6-7] FHDR.FCnt (frame counter)
    buf[7] = (frameCounterDown >> 8) & 0x00FF;
    //optional FHDR.Opts (FOptsLen in FCtrl)
#ifdef DEBUG
    Serial.print("+ MHDR FHDR:");
    for (int i = 0; i < 8; i++)
        Serial.printf(" %02x", buf[i]);
    Serial.println("");
#endif
    return 8;
}

int LoraWan::calculateMIC(char *buf, uint8_t size, bool direction, unsigned char *Key)
{
    unsigned char MIC_Data[200] = {0}; //TODO: actual size
    MIC_Data[0] = 0x49;
    // MIC_Data[1-4] = 0;
    MIC_Data[5] = direction;
    MIC_Data[6] = DevAddr[3];
    MIC_Data[7] = DevAddr[2];
    MIC_Data[8] = DevAddr[1];
    MIC_Data[9] = DevAddr[0];

    if (direction)
    {
        MIC_Data[10] = frameCounterDown & 0x00FF;
        MIC_Data[11] = (frameCounterDown >> 8) & 0x00FF;
    }
    else
    {
        MIC_Data[10] = frameCounterUp & 0x00FF;
        MIC_Data[11] = (frameCounterUp >> 8) & 0x00FF;
    }
    //MIC_Data[12-13] = 0; //Frame counter upper bytes
    //MIC_Data[14] = 0;
    MIC_Data[15] = size;
    for (uint8_t i = 0; i < size; i++) //copy the rest of the actual payload
    {
        MIC_Data[i + 16] = buf[i];
    }
    uint8_t mic_size = 16 + size;

    unsigned char Key_K1[16] = {0};
    unsigned char Key_K2[16] = {0};
    unsigned char New_Data[16] = {0};

    unsigned char Number_of_Blocks = mic_size / 16;
    unsigned char Incomplete_Block_Size = mic_size % 16;
    if (Incomplete_Block_Size != 0)
    {
        Number_of_Blocks++;
        MIC_Data[mic_size] = 0x80;
        for (int i = mic_size; i < (Number_of_Blocks * 16); i++)
        {
            MIC_Data[i] = 0;
        }
    }

    Generate_Keys(Key, Key_K1, Key_K2);
    for (uint8_t j = 0; j < (Number_of_Blocks - 1); j++)
    {
        //Copy new data and XOR with old
        for (uint8_t i = 0; i < 16; i++)
        {
            New_Data[i] ^= MIC_Data[(j * 16) + i];
        }
        AES_Encrypt(New_Data, Key);
    }
    if (Incomplete_Block_Size == 0)
    {
        //Copy last data into array
        for (uint8_t i = 0; i < 16; i++)
        {
            New_Data[i] ^= MIC_Data[((Number_of_Blocks - 1) * 16) + i];
            MIC_Data[i] ^= Key_K1[i];
        }
    }
    else
    {
        //Copy the remaining data and fill the rest
        for (uint8_t i = 0; i < 16; i++)
        {
            New_Data[i] ^= MIC_Data[((Number_of_Blocks - 1) * 16) + i];
            MIC_Data[i] ^= Key_K2[i];
        }
    }
    //Perform last AES routine
    AES_Encrypt(New_Data, Key);

    //TODO: Calculate MIC and append
    for (int8_t i = 0; i < 4; i++)
        buf[size + i] = New_Data[i];
    return size + 4;
}

void LoraWan::Generate_Keys(unsigned char *Key, unsigned char *K1, unsigned char *K2)
{
    unsigned char MSB_Key;
    AES_Encrypt(K1, Key);

    if ((K1[0] & 0x80) == 0x80)
    {
        MSB_Key = 1;
    }
    else
    {
        MSB_Key = 0;
    }

    //Shift K1 one bit left
    Shift_Left(K1);

    //if MSB was 1
    if (MSB_Key == 1)
    {
        K1[15] = K1[15] ^ 0x87;
    }

    //Copy K1 to K2
    for (char i = 0; i < 16; i++)
    {
        K2[i] = K1[i];
    }

    //Check if MSB is 1
    if ((K2[0] & 0x80) == 0x80)
    {
        MSB_Key = 1;
    }
    else
    {
        MSB_Key = 0;
    }

    //Shift K2 one bit left
    Shift_Left(K2);

    //Check if MSB was 1
    if (MSB_Key == 1)
    {
        K2[15] = K2[15] ^ 0x87;
    }
}

void LoraWan::Shift_Left(unsigned char *Data)
{
    unsigned char i;
    unsigned char Overflow = 0;
    //unsigned char High_Byte, Low_Byte;

    for (i = 0; i < 16; i++)
    {
        //Check for overflow on next byte except for the last byte
        if (i < 15)
        {
            //Check if upper bit is one
            if ((Data[i + 1] & 0x80) == 0x80)
            {
                Overflow = 1;
            }
            else
            {
                Overflow = 0;
            }
        }
        else
        {
            Overflow = 0;
        }

        //Shift one left
        Data[i] = (Data[i] << 1) + Overflow;
    }
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

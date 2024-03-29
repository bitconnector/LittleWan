#include "LoraWANmessage.hpp"

LoraWANmessage::LoraWANmessage() {}

LoraWANmessage::LoraWANmessage(unsigned char *_data)
{
    frameCounterDown = 0;
    frameCounterUp = 0;
    data = _data;
}

LoraWANmessage::~LoraWANmessage() {}

void LoraWANmessage::setDevAddr(const char *devAddr_in)
{
    DevAddr[0] = ASCII2Hex(&devAddr_in[3 * 2]);
    DevAddr[1] = ASCII2Hex(&devAddr_in[2 * 2]);
    DevAddr[2] = ASCII2Hex(&devAddr_in[1 * 2]);
    DevAddr[3] = ASCII2Hex(&devAddr_in[0 * 2]);
}

void LoraWANmessage::setNwkSKey(const char *NwkKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        NwkSKey[i] = ASCII2Hex(&NwkKey_in[i * 2]);
}

void LoraWANmessage::setAppSKey(const char *ApskKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        AppSKey[i] = ASCII2Hex(&ApskKey_in[i * 2]);
}

void LoraWANmessage::setFrameCounterUp(uint16_t up)
{
    frameCounterUp = up;
}

void LoraWANmessage::setFrameCounterDown(uint16_t down)
{
    frameCounterDown = down;
}

void LoraWANmessage::setAPB(const char *devAddr_in, const char *NwkKey_in, const char *ApskKey_in)
{
    setDevAddr(devAddr_in);
    setNwkSKey(NwkKey_in);
    setAppSKey(ApskKey_in);
}

void LoraWANmessage::setFRMPayload(uint8_t port, char *buf, uint8_t size)
{
    data[dataLen++] = port;
    for (int i = 0; i < size; i++)
        data[i + dataLen] = buf[i];

    if (size != 0)
        Encrypt_Payload(&data[dataLen], size, AppSKey, frameCounterUp);

    dataLen += size;
}

void LoraWANmessage::setMIC()
{
    calculateMIC((char *)data, dataLen, frameCounterUp, &data[dataLen]);
    dataLen += 4;
}

void LoraWANmessage::setMHDR(bool confirm)
{
    if (confirm)
        data[0] = 0x80; //confirmed uplink
    else
        data[0] = 0x40; //unconfirmed uplink
}

void LoraWANmessage::setFHDR(unsigned char FCtrl, char *FOpts)
{
    data[1] = DevAddr[0]; //[1-4] FHDR.DevAddr
    data[2] = DevAddr[1];
    data[3] = DevAddr[2];
    data[4] = DevAddr[3];
    data[5] = FCtrl;          //[5] FHDR.FCtrl (frame control)
    data[6] = frameCounterUp; //[6-7] FHDR.FCnt (frame counter)
    data[7] = frameCounterUp >> 8;

    unsigned char FOptsLen = FCtrl & 0x07; //copy FOpts in FHDR
    for (unsigned char i = 0; i < FOptsLen; i++)
        data[8 + i] = FOpts[i];

    dataLen = 8 + FOptsLen;
}

void LoraWANmessage::uplink(char *data, uint8_t len, uint8_t port, bool confirm)
{
    frameCounterUp++;
    setMHDR(confirm);
    setFHDR();
    setFRMPayload(port, data, len);
    setMIC();
}

char LoraWANmessage::getFtype(char data)
{
    return data & 0b11100000;
}

unsigned char LoraWANmessage::getFoptsLen(char data)
{
    return data & 0x07;
}

bool LoraWANmessage::checkHDR(char *data, uint8_t len)
{
    if (len < 11) //too small for a LoraWan packet
        return false;

    char mhdr = getFtype(data[0]);
    if (mhdr != 0x60 && mhdr != 0xA0) //unc down || conf down
        return false;                 //MHDR.FType doesnt match uplink

    unsigned char *dataDevAddr = (unsigned char *)&data[1];
    for (int i = 0; i < 4; i++)
        if (dataDevAddr[i] != DevAddr[i])
            return false; //wrong DevAddr

    uint16_t down = (data[7] << 8) | data[6];
    if (down < frameCounterDown)
        return false;            //message already recived
    frameCounterDown = down + 1; //get new counter value

    unsigned char mic[4];
    unsigned char *micData = (unsigned char *)&data[len - 4];
    calculateMIC(data, len - 4, down, mic, 1);
    for (int i = 0; i < 4; i++)
        if (mic[i] != micData[i])
            return false;

    //unsigned char payloadOffset = 8 + data[5] & 0x07; //get FOpts len

    return true;
}

unsigned char LoraWANmessage::ASCII2Hex(const char str[2])
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
        ASCII |= str[1] - 'A' + 10;
    else if (str[1] >= 'a' && str[1] <= 'f')
        ASCII |= str[1] - 'a' + 10;
    else
        ASCII |= str[1] - '0';
    return ASCII;
}

void LoraWANmessage::calculateMIC(char *data, uint8_t len, uint16_t counter, unsigned char *mic, bool direction)
{
    unsigned char MIC_Data[16] = {0};
    MIC_Data[0] = 0x49;
    // MIC_Data[1-4] = 0;
    MIC_Data[5] = direction; //direction 0=DOWN-link!!!
    MIC_Data[6] = DevAddr[0];
    MIC_Data[7] = DevAddr[1];
    MIC_Data[8] = DevAddr[2];
    MIC_Data[9] = DevAddr[3];
    MIC_Data[10] = counter;
    MIC_Data[11] = counter >> 8;
    //MIC_Data[12-13] = 0; //Frame counter upper bytes
    //MIC_Data[14] = 0;
    MIC_Data[15] = len;

    unsigned char Key_K1[16] = {0};
    unsigned char Key_K2[16] = {0};

    unsigned char Number_of_Blocks = len / 16;
    unsigned char Incomplete_Block_Size = len % 16;
    if (Incomplete_Block_Size != 0)
        Number_of_Blocks++;

    Generate_Keys(NwkSKey, Key_K1, Key_K2);
    unsigned char *key = Key_K2;

    AES_Encrypt(MIC_Data, NwkSKey);

    for (uint8_t j = 0; j < Number_of_Blocks; j++)
    {
        for (uint8_t i = 0; i < 16; i++) //Copy new data and XOR with old
        {
            uint8_t dataptr = (j * 16) + i;
            if (dataptr == len)
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
        AES_Encrypt(MIC_Data, NwkSKey);
    }

    for (int8_t i = 0; i < 4; i++)
        mic[i] = MIC_Data[i];
}

void LoraWANmessage::Generate_Keys(unsigned char *Key, unsigned char *K1, unsigned char *K2)
{
    unsigned char MSB_Key = 0;
    AES_Encrypt(K1, Key);

    if (K1[0] > 0x7F)
        MSB_Key = 1;

    Shift_Left(K1);

    if (MSB_Key == 1)
        K1[15] = K1[15] ^ 0x87;

    for (unsigned char i = 0; i < 16; i++)
        K2[i] = K1[i];

    if (K2[0] > 0x7F)
        MSB_Key = 1;
    else
        MSB_Key = 0;

    Shift_Left(K2);

    if (MSB_Key == 1)
        K2[15] ^= 0x87;
}

void LoraWANmessage::Shift_Left(unsigned char *Data)
{
    for (unsigned char i = 0; i < 15; i++)
    {
        if (Data[i + 1] > 0x7F) //Check if last upper bit is one == 0x80
            Data[i] = (Data[i] << 1) + 1;
        else
            Data[i] = (Data[i] << 1) + 0;
    }
    Data[15] <<= 1;
}

void LoraWANmessage::Encrypt_Payload(unsigned char *Buffer, unsigned char buffer_size, unsigned char *Key, uint16_t counter, bool direction)
{
    unsigned char j, i, idx;
    unsigned char Block_A[16];

    unsigned char Number_of_Blocks = buffer_size / 16; //Calculate number of blocks
    if ((buffer_size % 16) != 0)
        Number_of_Blocks++;

    for (i = 0; i < Number_of_Blocks; i++)
    {
        Block_A[0] = 0x01;
        Block_A[1] = 0;
        Block_A[2] = 0;
        Block_A[3] = 0;
        Block_A[4] = 0;

        Block_A[5] = direction;

        Block_A[6] = DevAddr[0];
        Block_A[7] = DevAddr[1];
        Block_A[8] = DevAddr[2];
        Block_A[9] = DevAddr[3];

        Block_A[10] = counter;
        Block_A[11] = counter >> 8;
        Block_A[12] = 0x00; //Frame counter upper Bytes
        Block_A[13] = 0x00;

        Block_A[14] = 0;

        Block_A[15] = i + 1;

        AES_Encrypt(Block_A, Key); //Calculate S

        for (j = 0; j < 16; j++)
        {
            idx = (i * 16) + j;
            if (buffer_size < idx)
                break;
            Buffer[idx] ^= Block_A[j];
        }
    }
}

#ifdef LITTLEWANDEBUGPORT
void printPackage(char *data, uint16_t size, bool structure)
{
    LITTLEWANDEBUGPORT.print("+ Package:");
    for (int i = 0; i < size; i++)
        LITTLEWANDEBUGPORT.printf(" %02x", data[i]);
    if (structure)
    {
        LITTLEWANDEBUGPORT.print("\n          |hd|   addr    |Ct| FCnt|");
        if (size < 13)
        {
            LITTLEWANDEBUGPORT.println("    MIC    |");
        }
        else if (size == 13)
        {
            LITTLEWANDEBUGPORT.println("Pt|    MIC    |");
        }
        else
        {
            LITTLEWANDEBUGPORT.print("Pt|");
            for (int i = 14; i < size; i++)
                LITTLEWANDEBUGPORT.print("   ");
            LITTLEWANDEBUGPORT.println("  |    MIC    |");
        }
    }
    else
        LITTLEWANDEBUGPORT.println();
}
#endif

#include "radio.hpp"

Radio::Radio()
{
    cs_pin = 7;
    spi = SPI;
}

Radio::Radio(int _cs)
{
    cs_pin = _cs;
    spi = SPI;
}

Radio::Radio(int _cs, SPIClass _spi)
{
    cs_pin = _cs;
    spi = _spi;
}

Radio::~Radio()
{
}

char Radio::readReg(char addr)
{
    char RFM_Data, RFM_Status;
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    RFM_Status = spi.transfer(addr);
    RFM_Data = SPI.transfer(0x00); //Send 0x00 to be able to receive the answer from the RFM
    digitalWrite(cs_pin, HIGH);
    spi.endTransaction();
#ifdef DEBUG
    Serial.print("# radio ");
    if (RFM_Status < 0x10)
        Serial.print("0");
    Serial.print(RFM_Status, HEX);
    Serial.print(" read: ");
    if (addr < 0x10)
        Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(" = ");
    if (RFM_Data < 0x10)
        Serial.print("0");
    Serial.print(RFM_Data, HEX);
#ifdef DEBUG_BIN
    Serial.print("(");
    for (char i = 0x80; i > RFM_Data; i >>= 1)
        Serial.print("0");
    if (RFM_Data)
        Serial.print(RFM_Data, BIN);
    Serial.print(")");
#endif
    Serial.println("");
#endif
    return RFM_Data;
}

void Radio::writeReg(char addr, char data)
{
    char RFM_Data, RFM_Status;
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    RFM_Status = spi.transfer(addr | 0x80); //Send Address with MSB 1 to make it a write command
    RFM_Data = spi.transfer(data);
    digitalWrite(cs_pin, HIGH);
    spi.endTransaction();
#ifdef DEBUG
    Serial.print("# radio ");
    if (RFM_Status < 0x10)
        Serial.print("0");
    Serial.print(RFM_Status, HEX);
    Serial.print(" writ: ");
    if (addr < 0x10)
        Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(" = ");
    if (data < 0x10)
        Serial.print("0");
    Serial.print(data, HEX);
#ifdef DEBUG_BIN
    Serial.print("(");
    for (char i = 0x80; i > data; i >>= 1)
        Serial.print("0");
    if (data)
        Serial.print(data, BIN);
    Serial.print(")");
#endif
    Serial.print(" -> ");
    if (RFM_Data < 0x10)
        Serial.print("0");
    Serial.print(RFM_Data, HEX);
    Serial.println("");
#endif
}

void Radio::writeState(Radio_State state)
{
    writeReg(1, (char)state);
}

void Radio::writeBinFrequency(uint32_t frequency)
{
    writeReg(6, frequency >> 16);
    writeReg(7, frequency >> 8);
    writeReg(8, frequency);
}

void Radio::writeFrequency(uint32_t frequency)
{
#ifdef DEBUG
    Serial.println("-> write frequency: " + String(frequency));
#endif
    float freq = frequency / 61.035;
    frequency = freq;
    writeBinFrequency(frequency);
}

void Radio::writeSendingParams(int sf, int pa, int bw)
{
    if (sf < 7)
        sf = 7;
    else if (sf > 10)
        sf = 10;

    if (pa < 0)
        pa = 0;
    else if (pa > 14)
        pa = 14;

    if (bw < 7)
        bw = 7;
    else if (bw > 8)
        bw = 8;

#ifdef DEBUG
    Serial.println("-> write sending params: pa=" + String(pa) + " sf=" + String(sf) + " bw=" + String(bw));
#endif
    writeReg(9, 0x70 | pa);           //powerlevel in dBm TODO: PA BOOST
    writeReg(12, 0x23);               //LNA boost on
    writeReg(30, (sf << 4) | 0b0100); //set SF and CRC on
    writeReg(29, (bw << 4) | 0x02);   //x kHz 4/5 coding rate explicit header mode
}

void Radio::sendPackage()
{
    writeState(Radio_State::STDBY); //switch to sending

    int _payload_size = 5;
    writeReg(34, _payload_size); //Set the Payload size

    char Data[] = "hallo";
    writeReg(13, 0x80);                     //Set the Pointer to the start of the tx FiFo
    for (int i = 0; i < _payload_size; i++) //Load the payload to the Radio
    {
        writeReg(0, Data[i]);
    }
    writeState(Radio_State::TX); //send the Packet
}

void Radio::readPackage()
{
    char Data[64];
    int size = readReg(0x13);
    unsigned char starting_addr = readReg(0x10);

    writeReg(13, starting_addr);
    for (int i = 0; i < size; i++)
        Data[i] = readReg(0);
}

byte Radio::readPackageSize()
{
    return readReg(0x13);
}

void Radio::clearIrqFlags(Radio_IRQ flags)
{
    writeReg(0x12, ~flags.raw); //clar all unset flags
}

Radio_State Radio::getState()
{
    return static_cast<Radio_State>(readReg(1));
}

int Radio::getModemState()
{
    unsigned char modemStatus = readReg(0x18);
    return modemStatus;
}

Radio_IRQ Radio::getIrqFlags()
{
    Radio_IRQ irq;
    irq.raw = readReg(0x12);
    return irq;
}

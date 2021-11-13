#include "radio.hpp"

Radio::Radio()
    : Radio(7) {}

Radio::Radio(int _cs)
    : Radio(_cs, SPI) {}

Radio::Radio(int _cs, SPIClass _spi)
{
    cs_pin = _cs;
    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);
    spi = _spi;
    spi.begin();
}

Radio::~Radio()
{
}

void Radio::resetWithPin(int reset_pin)
{
    pinMode(reset_pin, OUTPUT);
    digitalWrite(reset_pin, HIGH);
    delay(10);
    digitalWrite(reset_pin, LOW);
    delay(10);
    digitalWrite(reset_pin, HIGH);
    delay(50); //wait for radio to catch up
}

char Radio::readReg(char addr)
{
    char RFM_Data, RFM_Status;
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    RFM_Status = spi.transfer(addr);
    RFM_Data = spi.transfer(0x00); //Send 0x00 to be able to receive the answer from the RFM
    digitalWrite(cs_pin, HIGH);
    spi.endTransaction();
#ifdef DEBUG_SPI
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
#ifdef DEBUG_SPI_BIN
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
#ifdef DEBUG_SPI
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
#ifdef DEBUG_SPI_BIN
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

void Radio::writeBinFrequency(uint8_t frequency[3])
{
    writeReg(6, frequency[2]);
    writeReg(7, frequency[1]);
    writeReg(8, frequency[0]);
}

void Radio::writeFrequency(uint32_t frequency)
{
#ifdef DEBUG
    Serial.println("-> write frequency: " + String(frequency));
#endif
    float freq = frequency / 61.035;
    frequency = freq;
    writeBinFrequency((uint8_t *)(&frequency));
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

void Radio::sendPackage(char *data, uint8_t size)
{
#ifdef DEBUG
    Serial.println("-> start sending");
#endif
    writeState(Radio_State::STDBY); //switch to sending
    //in future switch channel,datarate

    writeReg(34, size); //Set the Payload size

    char txptr = readReg(14);
    writeReg(13, txptr); //Set the Pointer to the start of the tx FiFo
#ifdef DEBUG
    Serial.println("-> upload data");
#endif
    for (int i = 0; i < size; i++) //Load the payload to the Radio
    {
        writeReg(0, data[i]);
    }
#ifdef DEBUG
    Serial.println("-> upload data done starting tx");
#endif
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

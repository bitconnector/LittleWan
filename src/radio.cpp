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
    char RFM_Data;
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    spi.transfer(addr);
    RFM_Data = SPI.transfer(0x00); //Send 0x00 to be able to receive the answer from the RFM
    digitalWrite(cs_pin, HIGH);
    spi.endTransaction();
    return RFM_Data;
}

void Radio::writeReg(char addr, char data)
{
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    spi.transfer(addr | 0x80); //Send Address with MSB 1 to make it a write command
    spi.transfer(data);
    digitalWrite(cs_pin, HIGH);
    spi.endTransaction();
}

void Radio::writeState(Radio_State state)
{
    writeReg(1, (char)state);
}

void Radio::writeFrequency(uint32_t _frequency)
{
    float freq = _frequency / 61.035;
    _frequency = freq;
    Serial.println("-> write frequency: " + String(_frequency));
    writeReg(6, _frequency >> 16);
    writeReg(7, _frequency >> 8);
    writeReg(8, _frequency);
    // writeReg(6, 0xD9); //Channel [0], 868.1 MHz / 61.035 Hz = 14222987 = 0xD9068B
    // writeReg(7, 0x06);
    // writeReg(8, 0x8B);
}

void Radio::writeSendingParams()
{
    Serial.println("-> write sending params: pa=" + String(palevel) + " sf=" + String(spreading_factor) + " bw=" + String(bandwidth));
    writeReg(9, 0x70 | palevel);                    //powerlevel in dBm TODO: PA BOOST
    writeReg(12, 0x23);                             //LNA boost on
    writeReg(30, (spreading_factor << 4) | 0b0100); //set SF and CRC on
    writeReg(29, (bandwidth << 4) | 0x02);          //x kHz 4/5 coding rate explicit header mode
}

void Radio::writeBaseConfig()
{
    writeReg(38, 0x04); //Mobile node, low datarate optimization on AGC acorging to register LnaGain
    writeReg(31, 0x25); //Rx Timeout set to 37 symbols
    writeReg(32, 0x00); //set Preamble length set to 8 symbols
    writeReg(33, 0x08);
    writeReg(57, 0x34);   //Set LoRaWan sync word
    writeReg(0x0E, 0x80); //set tx Fifo addr
    writeReg(0x0F, 0x00); //set rx Fifo addr
}

void Radio::init()
{
    Serial.println("-> init Radio");
    writeState(Radio_State::STDBY); //enable lora mode (must be standby mode)
    writeBaseConfig();
    writeFrequency(0);
    writeSendingParams();
    //writeState(Radio_State::sleep); //shutdown the lora module
}

void Radio::setSF(int sf)
{
    if (sf < 7)
        sf = 7;
    else if (sf > 10)
        sf = 10;
    spreading_factor = sf;
}

void Radio::setPaLevel(int pa)
{
    if (pa < 0)
        pa = 0;
    else if (pa > 14)
        pa = 14;
    palevel = pa;
}

void Radio::setBW(int bw)
{
    if (bw < 7)
        bw = 7;
    else if (bw > 8)
        bw = 8;
    bandwidth = bw;
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

void Radio::reciveSinglePackage()
{
    writeState(Radio_State::RXSINGLE);
}

void Radio::reciveContinousPackage()
{
    writeState(Radio_State::RXCONTINUOUS);
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

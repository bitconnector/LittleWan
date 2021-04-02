#include "RadioConfig.hpp"
#include "radio.hpp"

RadioConfig::RadioConfig(Radio &_radio)
{
    radio = _radio;
}

RadioConfig::~RadioConfig()
{
}

void RadioConfig::sendMessage()
{
    //packMessage
    radio.sendPackage();
    //wait tx done
    radio.writeState(Radio_State::RXSINGLE);
    //wait rx done/timeout
    //evaluate message
}

void RadioConfig::writeBaseConfig()
{
    radio.writeReg(38, 0x04); //Mobile node, low datarate optimization on AGC acorging to register LnaGain
    radio.writeReg(31, 0x25); //Rx Timeout set to 37 symbols
    radio.writeReg(32, 0x00); //set Preamble length set to 8 symbols
    radio.writeReg(33, 0x08);
    radio.writeReg(57, 0x34);   //Set LoRaWan sync word
    radio.writeReg(0x0E, 0x80); //set tx Fifo addr
    radio.writeReg(0x0F, 0x00); //set rx Fifo addr
}

void RadioConfig::init()
{
    Serial.println("-> init Radio");
    radio.writeState(Radio_State::STDBY); //enable lora mode (must be standby mode)
    writeBaseConfig();
    radio.writeFrequency(0);
    radio.writeSendingParams(7, 14, 7);
    //writeState(Radio_State::sleep); //shutdown the lora module
}

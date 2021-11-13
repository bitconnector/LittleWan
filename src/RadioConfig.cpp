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
    const int _payload_size = 14;
    char data[_payload_size];

    data[0] = 0x40; //[0] MHDR -> unconfirmed uplink
    data[1] = 0x26; //[1-4] FHDR.DevAddr
    data[2] = 0x0B;
    data[3] = 0x70;
    data[4] = 0x92;
    data[5] = 0x00; //[5] FHDR.FCtrl (frame control)
    data[6] = 0x00; //[6-7] FHDR.FCnt (frame counter)
    data[7] = 0x01;
    //optional FHDR.Opts (FOptsLen in FCtrl)
    data[8] = 0x01;  //[8] FPort (1-255 -> AppSKey / 0 -> NwkSKey)
    data[9] = 0x23;  //[9] FRMPayload (not encrypted for now)
    data[10] = 0x00; //[10-13] MIC
    data[11] = 0x00;
    data[12] = 0x00;
    data[13] = 0x00;

    radio.sendPackage(data, _payload_size);
    //wait tx done
    //radio.writeState(Radio_State::RXSINGLE);
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
    radio.writeState(Radio_State::SLEEP); //enable lora mode (must be sleep mode)
    radio.writeState(Radio_State::STDBY); //switch to standby
    writeBaseConfig();
    radio.writeFrequency(868100000);
    radio.writeSendingParams(7, 14, 7);
    //writeState(Radio_State::sleep); //shutdown the lora module
}

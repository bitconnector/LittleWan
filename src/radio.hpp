#pragma once

#include <Arduino.h>
#include <SPI.h>

enum class Radio_State : byte
{
    SLEEP = 0x80,
    STDBY = 0x81,
    FSTX = 0x82,
    TX = 0x83,
    FSRX = 0x84,
    RXCONTINUOUS = 0x85,
    RXSINGLE = 0x86
};

union Radio_IRQ
{
    char raw;
    struct
    {
        byte CadDetected : 1;
        byte FhssChangeChannel : 1;
        byte CadDone : 1;
        byte TxDone : 1;
        byte ValidHeader : 1;
        byte PayloadCrcError : 1;
        byte RxDone : 1;
        byte RxTimeout : 1;
    } flags;
};

class Radio
{
public:
    Radio();
    Radio(int cs);
    Radio(int cs, SPIClass spi);
    ~Radio();

    //internal operations
    char readReg(char addr);
    void writeReg(char addr, char data);

    //translate settings onto Hardware
    void writeState(Radio_State state);
    void writeFrequency(uint32_t frequency);
    void writeSendingParams(int spreading_factor, int powerlevel, int bandwidth);
    // void writeBaseConfig();
    // void init(); //overwrite all settings needed

    //actions
    void sendPackage();
    void readPackage();
    void clearIrqFlags(Radio_IRQ); //probably already cleared by get Operation

    //get information
    Radio_State getState();
    int getModemState();
    Radio_IRQ getIrqFlags();
    byte readPackageSize();

protected:
    int cs_pin;
    SPIClass spi;

    // int spreading_factor = 7;       //SF7 -> 10
    // int palevel = 14;               //14 maximum with RFO
    // int bandwidth = 0x07;           //BW125 -> 0x08 BW250
    //uint32_t frequency = 868100000; //Frequency channel 1
};

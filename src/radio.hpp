/* Interface for the Radio to read and write its config.
*/

#pragma once

#include <Arduino.h>
#include <SPI.h>

#define DEBUG
#define DEBUG_SPI
#define DEBUG_SPI_BIN

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
    void writeBinFrequency(uint8_t frequency[3]);
    void writeSendingParams(int spreading_factor, int powerlevel, int bandwidth);

    //actions
    void sendPackage();
    void readPackage();
    void clearIrqFlags(Radio_IRQ); //probably already cleared by get Operation
    void resetWithPin(int reset_pin);

    //get information
    Radio_State getState();
    int getModemState();
    Radio_IRQ getIrqFlags();
    byte readPackageSize();

protected:
    int cs_pin;
    SPIClass spi;
};

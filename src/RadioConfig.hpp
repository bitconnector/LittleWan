/* Stores all config-parameters for the Radio to have the correct regional settings.
It also handles the different Radio states.
*/

#pragma once

#include "radio.hpp"

const uint16_t frequencys[] = {8681, 8683, 8685, 8671, 8673, 8675, 8677, 8679};

class RadioConfig
{
public:
    RadioConfig();
    RadioConfig(Radio &_radio);
    ~RadioConfig();

    void setSendParams();
    void setRecive2Params();

    void writeBaseConfig();
    void init();

    void sendMessage();
    void startRecive1();
    void startRecive2();
    void sleepRadio();
    //void resendMessage();

    bool gotPacket();
    //int messageType();
    int readMessage();
    int rfStatus();

protected:
    Radio radio;

    //tx/rx1
    int spreading_factor = 7; //SF7 -> 10
    int palevel = 14;         //14 maximum with RFO
    int bandwidth = 0x07;     //BW125 -> 0x08 BW250
    int frequency = 0;        // channel

    //rx2
    int spreading_factor2 = 7;       //SF7 -> 10
    int bandwidth2 = 0x07;           //BW125 -> 0x08 BW250
    uint32_t frequency2 = 869525000; //Frequency channel 1
};

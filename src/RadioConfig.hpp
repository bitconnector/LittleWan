#pragma once

#include "radio.hpp"

class RadioConfig
{
public:
    RadioConfig(Radio &_radio);
    ~RadioConfig();

    //void setKeys();
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
};

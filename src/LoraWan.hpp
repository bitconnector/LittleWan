#pragma once

#include "radio.hpp"

class LoraWan
{
public:
    LoraWan(Radio &_radio);
    ~LoraWan();

    void setKeys();
    void setSendParams();
    void setRecive2Params();

    void sendMessage();
    void startRecive1();
    void startRecive2();
    void sleepRadio();
    void resendMessage();

    bool gotPacket();
    int messageType();
    int readMessage();
    int rfStatus();

protected:
    Radio radio;
};

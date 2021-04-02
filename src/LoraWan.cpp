#include "LoraWan.hpp"
#include "radio.hpp"

LoraWan::LoraWan(Radio &_radio)
{
    radio = _radio;
}

LoraWan::~LoraWan()
{
}

void LoraWan::sendMessage()
{
    //packMessage
    radio.sendPackage();
    //wait tx done
    radio.writeState(Radio_State::RXSINGLE);
    //wait rx done/timeout
    //evaluate message
}

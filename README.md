# LittleWan

A [LoraWan](https://www.thethingsnetwork.org/) library build in layers and focused on simplicity.

| :warning: this library is still under heavy development! |
|----------------------------------------------------------|

If you dont want me to constantly crash your project, you have to select a release tag/version while importing the lib, because at this early stage the entire API is still subject to change

## working:

- otaa join
- apb uplink
  - generating uplink message
     - generate header
     - encrypt payload
     - calculate mic
  - sending uplink(LoRa.h)
- apb downlink
  - revicing RX1 downlink(LoRa.h)
  - check message
    - check header
    - validate mic


# LittleWan

A [LoraWan](https://www.thethingsnetwork.org/) library build in layers and focused on simplicity.

| :warning: this library is still under heavy development! |
|----------------------------------------------------------|

## working:

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


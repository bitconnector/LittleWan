#include <Arduino.h>

#include <SPI.h>
#include "LoRa.h"

// LoRa Pins
#define LoRa_RST 23
#define LoRa_CS 18
#define LoRa_DIO0 26
#define LoRa_DIO1 33
#define LoRa_DIO2 32
#define LoRa_SPI SPI

#include "LoraWANmessage.hpp"
unsigned char Buffer[235];
LoraWANmessage message = LoraWANmessage(Buffer);

void lora_tx(long frequency, int sf);
void lora_rx(long frequency, int sf);

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Starting"));
    esp_random();

    delay(30);
    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
    if (!LoRa.begin(868100000))
    {
        Serial.println("LORA faild init");
        while (1)
            ;
    }

    message.setDevAddr("260BCCD8");
    message.setNwkSKey("8D53AF486F7B992BE91A8193242554D7");
    message.setAppSKey("8CE6E6034DBAF7A01F7EAC95386B0C09");

    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
    if (!LoRa.begin(868100000))
    {
        Serial.println("LoRa init failed. Check your connections.");
        while (true)
            ; // if failed, do nothing
    }
}

void printPackage(char *data, uint16_t size, bool structure);
void onReceive(int packetSize);

void loop()
{
    char payload[10];
    sprintf(payload, "r=%02x", random(256));
    Serial.printf("Sending: %s\n", payload);

    lora_tx(868100000, 7);
    message.uplink(payload, strlen(payload), 5);
    printPackage((char *)message.data, message.dataLen, 1);

    LoRa.beginPacket(); // start packet
    LoRa.write(message.data, message.dataLen);
    LoRa.endPacket(); // finish packet and send it
    unsigned long txTime = millis();

    // RX1
    lora_rx(868100000, 7);
    while (txTime + 5000 > millis())
    {
        onReceive(LoRa.parsePacket());
    }
    Serial.println("end RX1");

    // RX2
    lora_rx(869525000, 12);
    while (txTime + 20000 > millis())
    {
        onReceive(LoRa.parsePacket());
    }
    Serial.println("end RX2");

    LoRa.sleep();
    sleep(60);
}

void onReceive(int packetSize)
{
    if (packetSize == 0)
        return; // if there's no packet, return

    Serial.print("Received ");

    char buf[200];
    int bufLen = 0;
    while (LoRa.available()) // read packet
    {
        buf[bufLen] = LoRa.read();
        bufLen++;
    }

    if (message.checkHDR(buf, bufLen))
        Serial.println("downlink");
    else
        Serial.println("some packet");
    printPackage(buf, bufLen, 1);
    Serial.print("with RSSI ");
    Serial.println(LoRa.packetRssi());
}

void lora_tx(long frequency, int sf)
{
    LoRa.begin(frequency);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(125E3);
}

void lora_rx(long frequency, int sf)
{
    LoRa.begin(frequency);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.disableCrc();
    LoRa.enableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(125E3);
}

void printPackage(char *data, uint16_t size, bool structure)
{
    Serial.print("+ Package:");
    for (int i = 0; i < size; i++)
        Serial.printf(" %02x", data[i]);
    if (structure)
    {
        Serial.print("\n          |hd|   addr    |Ct| FCnt|");
        if (size < 13)
        {
            Serial.println("    MIC    |");
        }
        else if (size == 13)
        {
            Serial.println("Pt|    MIC    |");
        }
        else
        {
            Serial.print("Pt|");
            for (int i = 14; i < size; i++)
                Serial.print("   ");
            Serial.println("  |    MIC    |");
        }
    }
    else
        Serial.println();
}

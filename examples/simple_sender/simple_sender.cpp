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
LoraWANmessage message = LoraWANmessage();

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Starting"));
    esp_random();

    message.setDevAddr("260BCCD8");
    message.setNwkSKey("8D53AF486F7B992BE91A8193242554D7");
    message.setAppSKey("8CE6E6034DBAF7A01F7EAC95386B0C09");

    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
}

void printPackage(char *data, uint16_t size, bool structure);

void loop()
{
    char payload[10];
    sprintf(payload, "r=%02x", random(256));
    Serial.printf("Sending: %s\n", payload);

    if (!LoRa.begin(868100000))
    {
        Serial.println("LoRa init failed. Check your connections.");
        while (true)
            ; // if failed, do nothing
    }
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);

    message.uplink(payload, strlen(payload), 5);
    printPackage((char *)message.data, message.dataLen, 1);

    LoRa.beginPacket(); // start packet
    LoRa.write(message.data, message.dataLen);
    LoRa.endPacket(); // finish packet and send it

    LoRa.sleep();
    sleep(60);
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

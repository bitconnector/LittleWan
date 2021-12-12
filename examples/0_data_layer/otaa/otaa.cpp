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

#include "LoraWANactivation.hpp"
unsigned char Buffer[235];
LoraWANmessage message = LoraWANmessage(Buffer);
LoraWANactivation otaa = LoraWANactivation(&message);

void printPackage(char *data, uint16_t size, bool structure);

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Starting"));
    esp_random();

    otaa.setDevNonce((uint16_t)random(256 * 256));
    otaa.setDevEUI("70B3D57ED0049D49");
    otaa.setJoinEUI("0000000000000000");
    otaa.setAppKey("ACBE2240FF67B00AB0377E547B65AA6B");

    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
    delay(100);
    LoRa.begin(868100000);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);

    Serial.print("Joining");
    bool joined = 0;
    while (!joined)
    {
        LoRa.begin(868100000);
        LoRa.enableCrc();
        LoRa.disableInvertIQ();

        otaa.setDevNonce((uint16_t)random(256 * 256));
        otaa.joinmsg();
        LoRa.beginPacket(); // start packet
        LoRa.write(message.data, message.dataLen);
        LoRa.endPacket(); // finish packet and send it
        unsigned long txTime = millis();

        //RX1
        LoRa.disableCrc();
        LoRa.enableInvertIQ();
        while (txTime + 10000 > millis() && !joined)
        {
            if (LoRa.parsePacket())
            {
                message.dataLen = 0;
                while (LoRa.available())
                    message.data[message.dataLen++] = LoRa.read();
                printPackage((char *)message.data, message.dataLen, 0);
                joined = otaa.checkJoin((char *)message.data, message.dataLen);
            }
        }
        Serial.print(".");
    }
    Serial.println("success!");
    LoRa.sleep();
}

void loop()
{
    LoRa.begin(868100000);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);

    char payload[100];
    sprintf(payload, "r=%02x", (int)random(256));
    Serial.printf("Sending: %s\n", payload);

    message.uplink(payload, strlen(payload), 5, 1);
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

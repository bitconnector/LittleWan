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

void randomSeedLora();

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Starting"));

    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
    if (!LoRa.begin(868100000))
    {
        Serial.println("LoRa init failed. Check your connections.");
        while (true)
            ; // if failed, do nothing
    }
}

void loop()
{
    Serial.printf("init %08lx", random(LONG_MAX));

    randomSeedLora();

    Serial.printf(" %08lx", random(LONG_MAX));
    Serial.println();
    delay(2000);
}

#define RANDOM_SEED_LORA_ITER 40
void randomSeedLora()
{
    LoRa.begin(868159800);
    LoRa.receive();
    for (int i = 0; i < RANDOM_SEED_LORA_ITER; i++)
    {
        long seedtmp = 0;
        for (int i = 0; i < 4; i++)
            seedtmp |= LoRa.random() << (i * 8);

        seedtmp ^= random(LONG_MAX);
        randomSeed((unsigned long)seedtmp);
    }
}

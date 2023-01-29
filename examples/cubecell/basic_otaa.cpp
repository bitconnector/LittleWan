#include "LoRaWan_APP.h"
#include "Arduino.h"

#include <LoraWANactivation.hpp>

#define RF_FREQUENCY 868100000 // Hz

#define TX_OUTPUT_POWER 20 // dBm

#define BUFFER_SIZE 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

void sendPackage(unsigned char *msg, unsigned char len, uint32_t freq);
void setReceive(uint32_t freq);

bool radioBusy = 0;

int16_t txNumber = 0;
int16_t Rssi = 0, rxSize = 0;

unsigned char Buffer[235];
LoraWANmessage message;
LoraWANactivation otaa;
bool joined = 0;

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

void setup()
{
  Serial.begin(115200);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);

  message = LoraWANmessage(Buffer);
  otaa = LoraWANactivation(&message);
  otaa.setDevEUI("70B3D57ED0059581");
  otaa.setJoinEUI("0000000000000000");
  otaa.setAppKey("D5125A5D9A89D819AA2AE471D86CAA62");

  // JOINING
  otaa.setDevNonce((uint16_t)random(256 * 256));
  delay(1000);

  while (!joined)
  {
    otaa.joinmsg();

    Serial.printf("sending join\n");
    printPackage((char *)message.data, message.dataLen, 0);

    uint32_t ti = millis() + 20000;
    sendPackage(message.data, message.dataLen, RF_FREQUENCY);
    while (radioBusy)
      Radio.IrqProcess();

    Serial.println("into RX mode");
    setReceive(RF_FREQUENCY);
    while (radioBusy && ti > millis())
      Radio.IrqProcess();

    if (!radioBusy)
      joined = otaa.checkJoin((char *)message.data, message.dataLen);
  }
}

void loop()
{
  delay(10000);

  txNumber++;

  sprintf(txpacket, "Hello world number %0.2f", txNumber); // start a package
  message.uplink((char *)txpacket, strlen(txpacket), 3, 0);

  Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));
  printPackage((char *)message.data, message.dataLen, 0);

  sendPackage(message.data, message.dataLen, RF_FREQUENCY);

  while (radioBusy)
    Radio.IrqProcess();
}

void OnTxDone(void)
{
  Serial.print("TX done......");
  radioBusy = 0;
}

void OnTxTimeout(void)
{
  Radio.Sleep();
  Serial.print("TX Timeout......");
  radioBusy = 0;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  Rssi = rssi;
  message.dataLen = size;
  memcpy(message.data, payload, size);
  Radio.Sleep();

  Serial.println("received packet");
  printPackage((char *)message.data, message.dataLen, 0);

  radioBusy = 0;
}

void sendPackage(unsigned char *msg, unsigned char len, uint32_t freq)
{
  Radio.SetChannel(freq);
  Radio.SetTxConfig(MODEM_LORA, 14, 0, 0,
                    8, 1,
                    8, false,
                    true, 0, 0, false, 3000);
  Radio.SetSyncWord(0x34);
  radioBusy = 1;
  Radio.Send((uint8_t *)msg, len);
}

void setReceive(uint32_t freq)
{
  Radio.SetChannel(freq);
  Radio.SetRxConfig(MODEM_LORA, 0, 8,
                    1, 0, 8,
                    0, false,
                    0, false, 0, 0, true, true);
  Radio.SetSyncWord(0x34);
  radioBusy = 1;
  Radio.Rx(0);
}

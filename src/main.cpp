#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5
#define RST_PIN 4
#define LED_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];

void setup() { 
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Lecteur initialise. En attente de badge..."));
}

void loop() {
  // Verifie la presence d'une nouvelle carte
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Lit l'identifiant de la carte
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Affiche l'UID / NUID sur le moniteur serie
  Serial.print(F("Badge detecte - NUID :"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Allumage de la LED bleue pendant 500 ms (GPIO 2)
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);

  // Termine la communication avec le badge
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
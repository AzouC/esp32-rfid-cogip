#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Broches du lecteur RC522 (VSPI) et LED
#define SS_PIN  5
#define RST_PIN 4
#define LED_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);

// Déclaration des constantes réseau et API
const char* WIFI_SSID = "private_CIEL";
const char* WIFI_PASS = "24Broce!!Fibre#CIEL";
const char* API_URL   = "http://192.168.1.7:8000/api/scan"; // À ajuster en Q26
const char* ZONE      = "Salle serveur";

/**
 * Fonction utilitaire pour extraire le NUID sous forme de String
 * à partir de la variable rfid.
 */
String nuidToString() {
  String s = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) s += "0";
    s += String(rfid.uid.uidByte[i], HEX);
  }
  s.toUpperCase();
  return s;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialisation du bus SPI et du lecteur RC522
  SPI.begin();
  rfid.PCD_Init();

  // Initialisation de la connexion Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connexion Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP ESP32 : ");
  Serial.println(WiFi.localIP());

  Serial.println("Pointeuse prête. En attente de badge...");
}

void loop() {
  // Vérifier la présence d'une carte et lire son identifiant
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  // Témoin visuel de lecture (LED intégrée GPIO 2)
  digitalWrite(LED_PIN, HIGH);

  // Extraction du NUID
  String nuid = nuidToString();
  Serial.println("NUID: " + nuid);

  // Envoi de la requête HTTP POST à l'API FastAPI
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"nuid\":\"" + nuid + "\",\"zone\":\"" + ZONE + "\"}";
    int code = http.POST(body);

    Serial.printf("HTTP %d %s\n", code, http.getString().c_str());
    http.end();
  }

  delay(500);
  digitalWrite(LED_PIN, LOW);

  // Réinitialiser la communication avec la carte
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
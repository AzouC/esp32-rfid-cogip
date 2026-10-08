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

// --- Configuration Réseau et Serveur ---
const char* WIFI_SSID = "SFR_F2CF";
const char* WIFI_PASS = "b43wf8svw9d5u3x4sz4s";
const char* API_URL   = "http://192.168.1.5:8000/api/scan";
const char* ZONE      = "Salle serveur";

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

  // Initialisation SPI et capteur RFID
  SPI.begin();
  rfid.PCD_Init();

  // Connexion Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connexion au Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[OK] Connecte !");
  Serial.print("IP ESP32 : ");
  Serial.println(WiFi.localIP());

  Serial.println("Pointeuse prete. En attente de badge...");
}

void loop() {
  // Detection et lecture du badge
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Signal visuel immediat
  digitalWrite(LED_PIN, HIGH);

  String nuid = nuidToString();
  Serial.println("\n--> Badge lu : " + nuid);

  // Transmission HTTP
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.setTimeout(3000);
    http.begin(client, API_URL);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"nuid\":\"" + nuid + "\",\"zone\":\"" + ZONE + "\"}";
    int code = http.POST(body);

    if (code > 0) {
      Serial.printf("[HTTP %d] %s\n", code, http.getString().c_str());
    } else {
      Serial.printf("[ERREUR] %s (Code : %d)\n", http.errorToString(code).c_str(), code);
    }

    http.end();
  }

  delay(500);
  digitalWrite(LED_PIN, LOW);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
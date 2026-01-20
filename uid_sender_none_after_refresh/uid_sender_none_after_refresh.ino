#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>

/* -------- WIFI CREDENTIALS -------- */
const char* ssid = "SSID";
const char* password = "PASSWORD";

/* -------- RFID PINS -------- */
#define SS_PIN 5
#define RST_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);
WebServer server(80);

/* -------- GLOBAL UID STORAGE -------- */
String lastUID = "";
bool cardAvailable = false;

/* -------- FUNCTION: READ RFID -------- */
void readRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  lastUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10)
      lastUID += "0";
    lastUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  lastUID.toUpperCase();
  cardAvailable = true;

  Serial.print("RFID UID Read: ");
  Serial.println(lastUID);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

/* -------- HTTP ENDPOINT -------- */
void handleUID() {
  if (cardAvailable) {
    server.send(200, "text/plain", lastUID);
    cardAvailable = false;   // clear after sending
    lastUID = "";
  } else {
    server.send(200, "text/plain", "NONE");
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("RFID Initialized");

  /* -------- WIFI CONNECT -------- */
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  /* -------- HTTP SERVER -------- */
  server.on("/uid", handleUID);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  readRFID();
}

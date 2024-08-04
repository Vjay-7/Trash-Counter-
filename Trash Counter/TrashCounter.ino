#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Define your WiFi and Firebase credentials
#define WIFI_SSID "PLDTHOMEFIBRkz267"
#define WIFI_PASSWORD "PLDTWIFIxqX36"
#define API_KEY "AIzaSyDqoS9NvRMxWaGW8UtnUTt-lgUWFDv9Z1g"
#define DATABASE_URL "https://trashbinpro-c6ff3-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define RFID pins
#define RST_PIN  4
#define SS_PIN   5

// Define button pins
#define BLUE_BUTTON_PIN 14
#define RED_BUTTON_PIN 12

// Define ultrasonic sensor pins
#define TRIG_PIN 27
#define ECHO_PIN 26

// Define servo pin
#define SERVO_PIN 13

// Create instances
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Servo servo;
FirebaseData firebaseData;

// Firebase configuration
FirebaseAuth auth;
FirebaseConfig config;

String namePath;
String pointsPath;
String name = "";
int points = 0;
String uid = "";

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  
  lcd.begin(16, 2);
  lcd.backlight();
  
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  servo.attach(SERVO_PIN);
  
  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp OK");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize RFID
  Serial.println("Scan an RFID card");
  if (!rfid.PCD_PerformSelfTest()) {
    Serial.println("MFRC522 initialization failed. Check connections and try again.");
    lcd.clear();
    lcd.print("RFID init failed");
    while (true); // Halt the program
  } else {
    Serial.println("MFRC522 initialized successfully.");
    lcd.clear();
    lcd.print("RFID initialized");
    lcd.print("Scan RFID card");
  }

  lcd.print("Scan RFID card");
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID card");
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    uid = getUID();
    Serial.print("UID tag: ");
    Serial.println(uid);
    
    namePath = "/users/" + uid + "/name";
    pointsPath = "/users/" + uid + "/points";
    name = "";
    points = 0;

    if (Firebase.RTDB.getString(&firebaseData, namePath)) {
      name = firebaseData.stringData();
      if (Firebase.RTDB.getInt(&firebaseData, pointsPath)) {
        points = firebaseData.intData();
      }
      
      lcd.clear();
      lcd.print("Welcome, " + name);
      delay(2000);
      lcd.clear();
    } else {
      lcd.clear();
      lcd.print("User not found");
      delay(2000);
      lcd.clear();
    }
  }

  if (uid != "" && digitalRead(BLUE_BUTTON_PIN) == LOW) {
    Serial.println("Press points");
    lcd.clear();
    lcd.print("Points: " + String(points));
    while (digitalRead(BLUE_BUTTON_PIN) == LOW) {
      delay(100); // Wait for button release
    }
    delay(3000); // Debounce delay
    lcd.clear();
  }

if (uid != "" && digitalRead(RED_BUTTON_PIN) == LOW) {
  Serial.println("Press open Bin");
  lcd.clear();
  lcd.print("Drop Bottles!");
  delay(1000);
  servo.write(180); // Open trash bin
  int newPoints = points;
  int bottleCount = 0;
  while (true) {
    if (digitalRead(RED_BUTTON_PIN) == LOW) break;
    else {
      int bottles = detectBottles();
      bottleCount += bottles;
      newPoints += bottles;
      if (bottles > 0) {
        lcd.setCursor(0, 1);
        lcd.print("Bottles: " + String(bottleCount));
      }
      delay(100); // Wait for potential bottle drops
    }
  }
  servo.write(5); // Close trash bin
  points = newPoints;
  Firebase.RTDB.setInt(&firebaseData, pointsPath, points);
  delay(1000); // Debounce delay
  lcd.clear();
}


  delay(200); // General debounce delay
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

int detectBottles() {
  int distance = measureDistance();
  Serial.println(distance);
  if (distance < 10) {  // Set threshold distance to 10 cm
    // Assuming a bottle is detected within 10 cm range
    delay(500); // Debounce bottle detection
    // Double-check to confirm bottle presence
    if (measureDistance() < 10) {
      return 1;
    }
  }
  return 0;
}

int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}

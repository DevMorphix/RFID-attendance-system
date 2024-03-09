#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


//add wifi crad
#define WIFI_SSID "master"
#define WIFI_PASSWORD "123456789"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDmwtA0ge0wmfuHfcbbhHlkRpaM4UupW84"

/* 3. Define the RTDB URL */
#define DATABASE_URL "pro-test-263ea-default-rtdb.asia-southeast1.firebasedatabase.app" 
//https://pro-test-263ea-default-rtdb.asia-southeast1.firebasedatabase.app/

//add mail id and password as given in the firebase 
#define USER_EMAIL "badhusha.shaji22@gmail.com"
#define USER_PASSWORD "123456789"

// RFID reader pins
#define SS_PIN 21
#define RST_PIN 5

// NTP client for getting current time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800); // UTC+5:30 offset

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the RFID reader
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
char str[32] = "";
String uid;

void setup()
{
    Serial.begin(115200);
    SPI.begin();

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Initialize Firebase
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);

    // Initialize RFID reader
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("Approximate your card to the reader...");
    Serial.println();

    // Initialize NTP client
    timeClient.begin();
}

void loop()
{
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
            String uid = "";
            for (byte i = 0; i < rfid.uid.size; i++) {
                uid.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
                uid.concat(String(rfid.uid.uidByte[i], HEX));
            }
            uid.toUpperCase(); // Make the UID uppercase

            Serial.print("UID Tag: ");
            Serial.println(uid);

            // Check if the UID is registered in Firebase
            if (Firebase.getInt(fbdo, "/users/" + uid)) {
                int status = fbdo.intData(); // 0 for checked out, 1 for checked in

                // Create a JSON object to store the RFID data
                FirebaseJson json;
                json.add("time", String(timeClient.getFormattedTime())); // Get the current time
                json.add("id", "Not Available"); // Replace with your device ID
                json.add("uid", uid);
                json.add("status", status == 0 ? 1 : 0); // Toggle the status

                // Update the user status in Firebase
                Firebase.setInt(fbdo, "/users/" + uid, status == 0 ? 1 : 0);

                // Send the JSON data to Firebase Realtime Database
                if (Firebase.pushJSON(fbdo, "/attendence", json)) {
                    Serial.println(fbdo.dataPath() + fbdo.pushName());
                } else {
                    Serial.println(fbdo.errorReason());
                }
            } else {
                Serial.println("New UID, registering in Firebase");
                // Register the new UID in Firebase
                Firebase.setInt(fbdo, "/users/" + uid, 1); // Set initial status as 1 (checked in)

                // Create a JSON object to store the RFID data for the new UID
                FirebaseJson json;
                json.add("time", String(timeClient.getFormattedTime())); // Get the current time
                json.add("id", "device11"); // Replace with your device ID
                json.add("uid", uid);
                json.add("status", 1); // Initial status is checked in

                // Send the JSON data to Firebase Realtime Database
                if (Firebase.pushJSON(fbdo, "/attendence", json)) {
                    Serial.println(fbdo.dataPath() + fbdo.pushName());
                } else {
                    Serial.println(fbdo.errorReason());
                }
            }
        }
    }

    delay(10);
}
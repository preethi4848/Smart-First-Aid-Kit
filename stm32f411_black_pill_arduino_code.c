#include <Arduino.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include<Notecard.h>

#define UART1_BAUD 115200  
#define UART2_BAUD 115200   
#define NOTE_CARRIER_ID "YOUR_CARRIER_ID" 
#define NOTE_AUTH_TOKEN "YOUR_AUTH_TOKEN" 


HardwareSerial notecardSerial(1);  
HardwareSerial nordicSerial(2);   

// Buffer sizes
const size_t bufferSize = 256;
char jsonBuffer[bufferSize];

void setup() {
    Serial.begin(115200);
    notecardSerial.begin(UART1_BAUD, SERIAL_8N1, 30, 31); 
    nordicSerial.begin(UART2_BAUD, SERIAL_8N1, 12, 13); 
    Serial.println("ESP32F411 initialized and ready to communicate with Notecard and Nordic.");

    
    initializeNotecard();
}

void loop() {
    
    if (notecardSerial.available()) {
        size_t len = notecardSerial.readBytesUntil('\n', jsonBuffer, sizeof(jsonBuffer) - 1);
        jsonBuffer[len] = '\0';

        Serial.print("Received JSON Data: ");
        Serial.println(jsonBuffer);

        
        processJsonData(jsonBuffer);
    }

    
    requestDataFromNotehub();
    delay(10000); 
}

void initializeNotecard() {
    
    sendCommand("{\"req\": \"hub.set\", \"token\": \"" NOTE_AUTH_TOKEN "\"}");
    sendCommand("{\"req\": \"card.set\", \"mode\": \"cellular\", \"pwr\": 1}");
}

void sendCommand(const char* command) {
    notecardSerial.println(command);
    Serial.print("Sent command: ");
    Serial.println(command);
}

void requestDataFromNotehub() {
    String request = "{\"req\": \"note.get\", \"hub\": \"" NOTE_CARRIER_ID "\"}";
    sendCommand(request.c_str());
}

void processJsonData(const char* jsonData) {
 
    DynamicJsonDocument doc(bufferSize);
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

  
    String box1 = doc["box1"].as<String>();
    String box2 = doc["box2"].as<String>();
    String box3 = doc["box3"].as<String>();

    Serial.print("Box1: ");
    Serial.println(box1);
    Serial.print("Box2: ");
    Serial.println(box2);
    Serial.print("Box3: ");
    Serial.println(box3);

   
    sendDataToNordic(box1, box2, box3);
}

void sendDataToNordic(const String& box1, const String& box2, const String& box3) {
   

    nordicSerial.println(box1);
    nordicSerial.println(box2);
    nordicSerial.println(box3);
    Serial.print("Sent data to Nordic: ");
    Serial.println(box1);
    Serial.println(box2);
    Serial.println(box3);
}

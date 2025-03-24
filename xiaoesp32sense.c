#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <esp_camera.h>
#include <EdgeImpulse.h>


const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* apiKey = "your_API_key";  

const char* speechToTextUrl = "https://speech.googleapis.com/v1p1beta1/speech:recognize?key=" + String(apiKey);
const char* ocrApiUrl = "http://your-ocr-api-endpoint";


camera_config_t camera_config = {
    .pin_pwdn = -1,
    .pin_reset = -1,
    .pin_xclk = 21,
    .pin_sccb_sda = 26,
    .pin_sccb_scl = 27,
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 33,
    .pin_d4 = 32,
    .pin_d3 = 39,
    .pin_d2 = 36,
    .pin_d1 = 23,
    .pin_d0 = 22,
    .pin_vsync = 25,
    .pin_href = 19,
    .pin_pclk = 18,
    .xclk_freq_hz = 20000000,
    .ledc_channel = LEDC_CHANNEL_0,
    .ledc_timer = LEDC_TIMER_0,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,
    .fb_count = 1
};


EdgeImpulse ei;
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
bool record_status = true;


#define UART1_BAUD 115200
HardwareSerial mySerial(1);

void connect_wifi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
}

void setup_camera() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera Init Failed: %s\n", esp_err_to_name(err));
        return;
    }
}

void setup_voice_recognition() {
    if (!ei.begin()) {
        Serial.println("Failed to initialize Edge Impulse!");
        while (1);
    }
}


String audioToBase64(const uint8_t *audio_data, size_t size) {
    
    return "";
}

void send_audio_to_server(const uint8_t *audio_data, size_t size) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(speechToTextUrl);
        http.addHeader("Content-Type", "application/json");

        String audioBase64 = audioToBase64(audio_data, size); 
        String jsonPayload = "{\"config\": {\"encoding\": \"LINEAR16\", \"sampleRateHertz\": 16000, \"languageCode\": \"en-US\"}, \"audio\": {\"content\": \"" + audioBase64 + "\"}}";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Speech-to-Text Response:");
            Serial.println(response);

          
            String recognized_text = extract_text_from_response(response);

            
            mySerial.print("Speech: ");
            mySerial.println(recognized_text);
        } else {
            Serial.printf("Error on HTTP request: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

String extract_text_from_response(const String& response) {
  
    return "";
}

void send_image_to_server(const uint8_t *image_data, size_t size) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(ocrApiUrl);
        http.addHeader("Content-Type", "image/jpeg");
        int httpResponseCode = http.POST(image_data, size);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("OCR Response:");
            Serial.println(response);

            
            mySerial.print("OCR: ");
            mySerial.println(response);
        } else {
            Serial.println("Error on HTTP request");
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

void setup() {
    Serial.begin(115200);
    connect_wifi();
    setup_camera();
    setup_voice_recognition();

    
    mySerial.begin(UART1_BAUD, SERIAL_8N1, 16, 17); 
}

void loop() {
  
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    Serial.println("Captured image, sending to server...");
    send_image_to_server(fb->buf, fb->len);
    esp_camera_fb_return(fb);

    
    uint8_t audio_data[1024]; 
    size_t audio_size = sizeof(audio_data);

   
    send_audio_to_server(audio_data, audio_size);

    delay(10000); 
}

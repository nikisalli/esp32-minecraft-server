#include <Arduino.h>
#include <WiFi.h>
#include <minecraft.h>
#include <credentials.h>

TaskHandle_t listener;
WiFiServer server(server_port);

int timeoutTime = 2000;

void update( void * pvParameters ){
    // mc.handle();
}

void setup() {
    disableCore0WDT();
    disableCore1WDT();
    disableLoopWDT();

    Serial.begin(115200);
    delay(100);
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());

    delay(1000);

    server.begin();
    Serial.println("[INFO] server started");
    // xTaskCreatePinnedToCore(update, "listener", 100000, NULL, 2, &listener, 0);
}

void loop(){
    WiFiClient client = server.available();

    if(client){
        long currentTime = millis();
        long previousTime = currentTime;
        Serial.println("[INFO] client connected");
        minecraft mc (&client);
        while (client.connected() && currentTime - previousTime <= timeoutTime) {
        // while (true) {
            mc.handle();
        }
    }
}
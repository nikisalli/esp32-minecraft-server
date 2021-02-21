#include <Arduino.h>
#include <WiFi.h>
#include <minecraft.h>
#include <credentials.h>
#include <memory>

#define MAX_CLIENTS 2

TaskHandle_t listener;
WiFiServer server(server_port);
WiFiClient serverClients[MAX_CLIENTS];

int timeoutTime = 2000;

void playerHandler0( void * parameter ){
    Serial.println("hello from task 0");
    uint32_t currentTime = millis();
    uint32_t previousTime = currentTime;
    minecraft mc ((WiFiClient*)parameter, 0);
    if(!mc.join()){
        (*(WiFiClient*)parameter).stop();
        vTaskDelete(NULL);
    }
    while ((*(WiFiClient*)parameter).connected() && currentTime - previousTime <= 2000) {
    // while (true) {
        mc.handle();
    }
    vTaskDelete(NULL);
}

void playerHandler1( void * parameter ){
    Serial.println("hello from task 1");
    uint32_t currentTime = millis();
    uint32_t previousTime = currentTime;
    minecraft mc ((WiFiClient*)parameter, 1);
    if(!mc.join()){
        (*(WiFiClient*)parameter).stop();
        vTaskDelete(NULL);
    }
    while ((*(WiFiClient*)parameter).connected() && currentTime - previousTime <= 2000) {
    // while (true) {
        mc.handle();
    }
    vTaskDelete(NULL);
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
}

void loop(){
    uint8_t i;
    //check if there are any new clients
    if (server.hasClient()){
        for(i = 0; i < MAX_CLIENTS; i++){
        //find free/disconnected spot
            if (!serverClients[i] || !serverClients[i].connected()){
                if(serverClients[i]) serverClients[i].stop();
                serverClients[i] = server.available();
                Serial.print("[INFO] New client connected: "); Serial.println(i);
                char name[20];
                snprintf(name, 20, "playerHandler%d", i);
                if(i == 0){
                    xTaskCreatePinnedToCore(playerHandler0, name, 50000, (void*)&serverClients[i], 2, NULL, 0);
                } else if(i == 1){
                    xTaskCreatePinnedToCore(playerHandler1, name, 50000, (void*)&serverClients[i], 2, NULL, 1);
                }
                return;
            }
        }
        //no free/disconnected spot so reject
        WiFiClient serverClient = server.available();
        serverClient.stop();
        Serial.println("[INFO] server is full!");
    }
}

/*void loop() {
    uint8_t i;
    //check if there are any new clients
    if (server.hasClient()){
        for(i = 0; i < MAX_CLIENTS; i++){
        //find free/disconnected spot
            if (!serverClients[i] || !serverClients[i].connected()){
                if(serverClients[i]) serverClients[i].stop();
                serverClients[i] = server.available();
                Serial.print("[INFO] New client connected: "); Serial.println(i);
                return;
            }
        }
        //no free/disconnected spot so reject
        WiFiClient serverClient = server.available();
        serverClient.stop();
        Serial.println("[INFO] server is full!");
    }

    //check clients for data
    for(i = 0; i < MAX_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()){
            if(serverClients[i].available()){
                //get data from the telnet client and push it to the UART
                while(serverClients[i].available()){
                    Serial.write(serverClients[i].read());
                    Serial.print(i);
                }
            }
        }
    }
    //check UART for data
    if(Serial.available()){
        size_t len = Serial.available();
        uint8_t sbuf[len];
        Serial.readBytes(sbuf, len);
        //push UART data to all connected telnet clients
        for(i = 0; i < MAX_CLIENTS; i++){
            if (serverClients[i] && serverClients[i].connected()){
                serverClients[i].write(sbuf, len);
                delay(100);
            }
        }
    }
}*/

/*void loop(){
    WiFiClient client = server.available();

    if(client){
        uint32_t currentTime = millis();
        uint32_t previousTime = currentTime;
        Serial.println("[INFO] client connected");
        minecraft mc (&client);
        if(!mc.join()){
            client.stop();
            return;
        }
        while (client.connected() && currentTime - previousTime <= CLIENT_TIMEOUT_TIME) {
        // while (true) {
            mc.handle();
        }
    }
}*/
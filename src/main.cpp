#include <ESP8266WiFi.h>
#include "main.h"
#include "lcd.h"
#include "html.h"
#include "parsing.h"

#define DEBUG

struct SSEClient {
    uint32_t msgCount;
    std::unique_ptr<WiFiClient> client;
};

WiFiServer server(80);
//std::unique_ptr<WiFiClient> clients[10];
SSEClient clients[10];

volatile uint8_t clientCount = 0;

void displayClientCount(void) {
    static uint8_t prevCount = 255;

    if (prevCount == clientCount)
        return;

    char clientCountBuf[20];

    snprintf(
        clientCountBuf, 
        20, 
        (clientCount < 10 ? "Client count: 0%d" : "Client count: %d"),
        clientCount
    );

    lcd_writeLines("", clientCountBuf, false);
    prevCount = clientCount;
}


void handleClient(void) {
    static char buf[REQ_SZLIMIT];
    auto client = std::make_unique<WiFiClient>(server.accept());
    if (!(*client))
        return;

    // Wait for client to write. We do this properly later
    while (!client->available())
        yield();

    memset(buf, 0, REQ_SZLIMIT);
    client->read(buf, REQ_SZLIMIT);
    volatile uint16_t requestLen = strnlen(buf, REQ_SZLIMIT);

    #ifdef DEBUG
    Serial.print("Request length: ");
    Serial.println(requestLen);
    Serial.println(buf);
    Serial.println("==============");
    #endif

    // Request length being equal to the buffer size means no tull terminating
    // character was found withing the buffer and thus we ignore the request
    if (REQ_SZLIMIT == requestLen) {
        client->write_P(
            PSTR(
                "HTTP/1.1 413 Request Entity Too Large\n"
                "Content-Type: application/json\n\n"
                "{\"error\": \"request size limit exceeded\"}"
            ),
            110
        );

        client->flush();
        return;
    }

    Request request = parseRequest(buf);

    #ifdef DEBUG
    Serial.println(request.method);
    Serial.println(request.endpoint);
    Serial.println(request.body);
    #endif

    if (strstr(buf, "/events") != nullptr)
        handleSubscription(std::move(client));
    else
        handleRequest(std::move(client));
}


void handleRequest(std::unique_ptr<WiFiClient> client) {
    client->write("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
    client->write_P(indexHtml, lenIndexHtml);
    client->flush();
}


void handleSubscription(std::unique_ptr<WiFiClient> client) {
    for (auto& slot: clients) {
        if (slot.client == nullptr) {
            slot = {0, std::move(client)};
            slot.client->write("HTTP/1.1 200 OK\nCache-Control: no-store\nContent-Type: text/event-stream\n\n");
            clientCount++;
            break;
        }
    }
}


void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    pinMode(LED_BUILTIN, OUTPUT);
    lcd_init();
    WiFi.mode(WIFI_STA);
    WiFi.begin("DNA-Mokkula-2G-HT0RH1", "Pasikissa2x1");

    while (WiFi.status() != WL_CONNECTED) {
        for (int i = 0; i < 4; i++) {
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
        }
    }

    char addressBuf[20];
    WiFi.localIP().toString().toCharArray(addressBuf, 20);
    lcd_writeLines(addressBuf);

    server.begin();

    //timer1_attachInterrupt(displayClientCount);
    //timer1_write(31250);
    //timer1_enable(2, 0, 1);
}


void loop() {
    digitalWrite(LED_BUILTIN, LOW);

    for (auto& slot: clients) {
        // Slot is empty, keep going
        if (slot.client == nullptr)
            continue;

        // Slot wasn't empty but client is gone, clear slot
        if (!slot.client->availableForWrite()) {
            slot.client->stop();
            slot.client = nullptr;
            clientCount--;
            continue;
        }

        char msgBuf[200];
        snprintf(msgBuf,
            200,
            "data: {\"messageCount\":%d,\"clientCount\":%d,\"runTime\":%lu}\n\n",
            ++slot.msgCount,
            clientCount,
            millis()
        );

        slot.client->write(msgBuf);
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    handleClient();
    displayClientCount();
    yield();
}

#ifndef WEMOS_SSE_H
#define WEMOS_SSE_H

#include <memory>
#include <ESP8266WiFi.h>

constexpr uint16_t REQ_SZLIMIT = 500;

void displayClientCount(void);
void handleClient(void);
void handleRequest(std::unique_ptr<WiFiClient> client);
void handleSubscription(std::unique_ptr<WiFiClient> client);

#endif

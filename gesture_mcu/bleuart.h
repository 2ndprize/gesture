#ifndef BLEUART_H
#define BLEUART_H

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

extern BLEDfu  bledfu;  // OTA DFU service
extern BLEDis  bledis;  // device information
extern BLEUart bleuart; // uart over ble
extern BLEBas  blebas;  // battery

void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void startAdv(void);
void bleUART(int kind);

#endif  // BLEUART_H
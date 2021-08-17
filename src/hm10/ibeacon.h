#ifndef IBEACON_HM10_H
#define IBEACON_HM10_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "../memory/memory_types.h"
#include "../led/led.h"

#define BT_SCAN_FREQ 15000
#define UUID "74278BDAB"

#define DEBUG_SERIAL true
#define DISCOVERY_CACHE 10

class BtScanInterpreter {
public:
    char buf[6] = "12345";

    char uuid[33];
    char part2[11];
    char rssi[8];

    boolean inMessage = false;
    boolean complete = false;
    boolean goodMessage = false;
    int numberColons;
    int writePos;

    void initialize();
    bool accept(char ch);
};


class BLEDevice {
public:
    SoftwareSerial* _serial;

    char beaconChar[4];
    int beaconMinor;

    int discovery[DISCOVERY_CACHE] = {0};
    byte discoveryIdx;

    LEDModule led;

    BLEDevice(SoftwareSerial* ss);
    void setupIScanner();
    void bleScan();
    void setupIBeacon();
    void setMinorNum(char minor[]);

    // Can be overwritten
    void processBeaconResult(TraceRecord* tr) __attribute__((weak));

private:
    void sendATCommand(char* cmd, byte resLen);
    bool convertBeaconResult(char* data, char* rssi);
    TraceRecord createTraceRecord(int minor, float distance);

};


#endif
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#include "src/gps/gps.h"
#include "src/hm10/ibeacon.h"
#include "src/memory/memory.h"
#include "src/nfc/pn532_nfc.h"
#include "src/nfc/libs/snep.h"
#include "src/nfc/libs/PN532_SPI.h"

#define DO_GPS true
#define DO_IBEACON true
#define DO_BLE_SCAN true
#define DO_MEMORY_READ false
#define DO_NFC true

#define DEBUG_MAIN false

// Timing variables
unsigned long gpsScanTime;
unsigned long bleScanTime;

// Interrupt
const byte interruptPin = 3;

// GPS Variables
#if DO_GPS
    GPSModule gpsModule;

    char nmeaBuffer[NMEA_BUFFER_LEN];
    MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
#endif

// BLE variables
char beaconMinor[5] = "0000";

#if DO_IBEACON
    SoftwareSerial beaconSS(6,7);
    BLEDevice iBeacon(&beaconSS);
#endif

#if DO_BLE_SCAN
    SoftwareSerial scannerSS(8,9);
    BLEDevice iScanner(&scannerSS);
#endif

// Memory Variables
Memory memory(0x50);

// NFC Variables
#if DO_NFC
    PN532_SPI pn532spi(SPI,10);
    SNEP nfcSNEP(pn532spi);

    NFCModule nfc(&nfcSNEP);
#endif

// Main Variables
#define START_MODE 1
#define OPER_MODE 2
#define FINISH_MODE 3

volatile byte mode = START_MODE;

void finish() {
    mode = FINISH_MODE;
}

void setup() {
    // Open serial communications and wait for port to open:
    Wire.begin();
    Serial.end();
    Serial.begin(9600);

     while (!Serial) {
         ; // wait for serial port to connect. Needed for Native USB only
     }

    Serial.println(F("Starting program"));

    // Setup interrupt
    pinMode(interruptPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptPin), finish, FALLING);


//    #if DO_IBEACON
//        //Serial.println(F("iBeacon Started"));
//        iBeacon.setupIBeacon();
//    #endif

    #if DO_BLE_SCAN
        //Serial.println(F("iScanner Started"));
//        #if DO_IBEACON
//            delay(2000);
//        #endif
//
//        iScanner.setupIScanner();
        bleScanTime = millis();
    #endif

    #if DO_GPS
        //Serial.println(F("GPS Started"));
        gpsModule.begin();
        gpsScanTime = millis();
    #endif

    #if DO_MEMORY_READ
        long currentAddress = 0;
        long newAddress = 0;

        do {
            currentAddress = newAddress;
            Serial.println(currentAddress);
            newAddress = memory.debugReadMemory(currentAddress);
        } while (currentAddress != newAddress);
    #endif

}

void loop() {

    if (mode == START_MODE) {
        #if DO_NFC
            bool rx = nfc.receiveEmployeeID(beaconMinor);

            if (rx) {
                Serial.println(beaconMinor);
                
                #if DO_IBEACON
                    //Serial.println(F("iBeacon Started"));
                    iBeacon.setMinorNum(beaconMinor);
                    iBeacon.setupIBeacon();
                #endif

                #if DO_BLE_SCAN
                    #if DO_IBEACON
                        delay(1000);
                    #endif
                
                    iScanner.setMinorNum(beaconMinor);
                    iScanner.setupIScanner();
                    bleScanTime = millis();
                #endif

                #if DO_GPS
                    //Serial.println(F("GPS Started"));
                    gpsScanTime = millis();
                #endif

                mode = OPER_MODE; // Replace me
            } else {
                delay(3000);
            }
        #endif
    }

    if (mode == OPER_MODE) {
        #if DO_GPS
            // GPS Scanning
            if (millis() - gpsScanTime >= GPS_SCAN_FREQ) {
                gpsScanTime = millis();
                GPSRecord gpsr = gpsModule.getGPS(&nmea);
    
                Serial.println(F("Found GPS"));
    
                memory.writeGPSRecord(&gpsr);
            }
        #endif
    
    
        #if DO_BLE_SCAN
            if (millis() - bleScanTime >= BT_SCAN_FREQ) {
                bleScanTime = millis();
                iScanner.bleScan();
            }
        #endif
    }

    if (mode == FINISH_MODE) {
        #if DO_NFC
            bool rxf = nfc.transferData(&memory);

            if (rxf) {
                Serial.println(F("Finished"));
                mode = 4;
            }
        #endif
    }
}

/*
*  Process NMEA data from the Ublox GPS
*/
#if DO_GPS
    void UBLOX_GPS::processNMEA(char incoming) {
      nmea.process(incoming);
    }
#endif

/*
*  Overwrite processBeaconResult to write to memory
*/
#if DO_BLE_SCAN
   void BLEDevice::processBeaconResult(TraceRecord* tr) {
       memory.writeTraceRecord(tr);
   }
#endif

/**************************************************************************/
/*!
    @file     PN532.h
    @author   Adafruit Industries & Seeed Studio
    @license  BSD
*/
/**************************************************************************/

#ifndef __PN532_H__
#define __PN532_H__

#include "PN532Interface.h"
#include <Arduino.h>

// PN532 Commands
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)


class PN532 {
public:
    PN532(PN532Interface &interface);

    void begin(void);

    // Generic PN532 functions
    bool SAMConfig(void);
    uint32_t getFirmwareVersion(void);

    /**
    * @brief    Init PN532 as a target
    * @param    timeout max time to wait, 0 means no timeout
    * @return   > 0     success
    *           = 0     timeout
    *           < 0     failed
    */
    int8_t tgInitAsTarget(uint16_t timeout = 0);
    int8_t tgInitAsTarget(const uint8_t* command, const uint8_t len, const uint16_t timeout = 0);

    int16_t tgGetData(uint8_t *buf, uint8_t len);
    bool tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

    uint8_t *getBuffer(uint8_t *len) {
        *len = sizeof(pn532_packetbuffer) - 4;
        return pn532_packetbuffer;
    };

private:
    uint8_t pn532_packetbuffer[64];

    PN532Interface *_interface;
};

#endif

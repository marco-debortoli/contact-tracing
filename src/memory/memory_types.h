#ifndef CONTACT_MEMORY_TYPES_H
#define CONTACT_MEMORY_TYPES_H

#include <Arduino.h>

#define TRACE_RECORD_ID 0x01
#define GPS_RECORD_ID 0x02
#define GARBAGE_RECORD_ID 0x03
#define MEM_SEPARATOR 0x2C

#define GPS_RECORD_LEN 17
#define TRACE_RECORD_LEN 15

typedef union {
    float number;
    byte bytes[4];
} float_t;


typedef union {
    unsigned long timestamp;
    byte bytes[4];
} timestamp_t;


typedef union {
    int eID;
    byte bytes[4];
} employee_id_t;


/*
*  To record a Bluetooth trace record in memory
*/
struct TraceRecord {
    timestamp_t timestamp;
    float_t distance;
    employee_id_t employeeID;
};


/*
*  To record a GPS record in memory
*/
struct GPSRecord {
	timestamp_t timestamp;
	float_t latitude;
	float_t longitude;
};



#endif
#ifndef CONTACT_TRACING_GPS_H
#define CONTACT_TRACING_GPS_H

#include <Arduino.h>
#include <MicroNMEA.h>

#include "ublox.h"
#include "../memory/memory_types.h"
#include "../led/led.h"

#define MAX_GPS_COUNT 3
#define NMEA_BUFFER_LEN 85
#define GPS_SCAN_FREQ 60000


class GPSModule {
public:
	UBLOX_GPS myGPS;

	LEDModule led;

	bool begin();
	GPSRecord getGPS(MicroNMEA* nmea);

	GPSModule();

private:
	byte _currentLatIdx;
	byte _currentLonIdx;

	float _longitudes[MAX_GPS_COUNT];
	float _latitudes[MAX_GPS_COUNT];

	void saveGPS(long latitude, long longitude);

	bool inLatList(float value);
	bool inLonList(float value);

	float averageLat();
	float averageLon();
};


#endif
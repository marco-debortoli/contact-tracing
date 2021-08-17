#ifndef CONTACT_TRACING_LED_H
#define CONTACT_TRACING_LED_H

#include <Arduino.h>

#define MAX_GPS_COUNT 3
#define NMEA_BUFFER_LEN 85
#define GPS_SCAN_FREQ 60000


class LEDModule {
public:
	LEDModule(int pin);

	void quickBlink();
	void mediumBlink();
	void longBlink();
	void on();
	void off();


private:
	int _ledPin;
};


#endif
#ifndef UBLOX_GPS_LIBRARY_H
#define UBLOX_GPS_LIBRARY_H

#include <Arduino.h>
#include <Wire.h>


//I2C_BUFFER_LENGTH is defined in Wire.H
#define I2C_BUFFER_LENGTH 32


class UBLOX_GPS {
public:
	UBLOX_GPS(void);

	//By default use the default I2C address, and use Wire port
	boolean begin(TwoWire &wirePort = Wire, uint8_t deviceAddress = 0x42); //Returns true if module is detected

	//Returns true if device answers on _gpsI2Caddress address or via Serial
	//maxWait is only used for Serial
	boolean isConnected(uint16_t maxWait = 1100);

	boolean checkUblox();
	boolean checkUbloxI2C();
	void process(uint8_t incoming);	//Processes NMEA binary sentences one byte at a time
	void processNMEA(char incoming) __attribute__((weak));

	void setNMEAOutputPort(Stream &nmeaOutputPort);

private:
	// Depending on the sentence type the processor will load characters into different arrays
	enum SentenceTypes
	{
		NONE = 0,
		NMEA,
		UBX,
		RTCM
	} currentSentence = NONE;

	enum commTypes
	{
		COMM_TYPE_I2C = 0,
		COMM_TYPE_SERIAL,
		COMM_TYPE_SPI
	} commType = COMM_TYPE_I2C;

	// Variables
	TwoWire *_i2cPort;
	Stream *_nmeaOutputPort = NULL;

	uint8_t _gpsI2Caddress = 0x42; //Default 7-bit unshifted address of the ublox 6/7/8/M8/F9 series
	uint8_t i2cPollingWait = 100; //Default to 100ms. Adjusted when user calls setNavigationFrequency()

	unsigned long lastCheck = 0;

};

#endif

#ifndef CONTACT_MEMORY_H
#define CONTACT_MEMORY_H

#include <Wire.h>
#include <Arduino.h>
#include "memory_types.h"

#define DEBUG_MEMORY true


class Memory {
public:
	Memory(int deviceAddress);

	TraceRecord readTraceRecord(long startAddress);
	void writeTraceRecord(TraceRecord* tr);

	GPSRecord readGPSRecord(long startAddress);
	void writeGPSRecord(GPSRecord* gr);
	long readNextByteRecord(long startAddress, uint8_t* byteArr);
	void writeByte(byte by);

	#if DEBUG_MEMORY
		long debugReadMemory(long startAddress);
	#endif

private:
	long _currentAddress = 0;
	int _deviceAddress = 0x50;
};


#endif
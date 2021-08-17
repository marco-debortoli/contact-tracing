#include "memory.h"

Memory::Memory(int deviceAddress) {
	_currentAddress = 0;
	_deviceAddress = deviceAddress;
}


void Memory::writeByte(byte by) {
	Wire.beginTransmission(_deviceAddress);

	Wire.write((int)(_currentAddress >> 8)); // MSB
	Wire.write((int)(_currentAddress & 0xFF)); // LSB  

	Wire.write(by);

	Wire.endTransmission();

	_currentAddress++;

	// So inefficient - better to use page TODO
	delay(10);
}



/*
*  Reading and Writing TraceRecords to memory
*
*/

TraceRecord Memory::readTraceRecord(long startAddress) {
	float_t distance;
	employee_id_t employee;
	timestamp_t ts;
	TraceRecord res;

	byte counter = 0;

	Wire.beginTransmission(_deviceAddress);
	Wire.write((int)(startAddress >> 8)); // MSB
	Wire.write((int)(startAddress & 0xFF)); // LSB 
	Wire.endTransmission();

	// READ THE TYPE
	Wire.requestFrom(_deviceAddress, 1);
	byte rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	if (rdata != TRACE_RECORD_ID) {
		return res;
	}

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// Timestamp
	while (counter < sizeof(unsigned long)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		ts.bytes[counter] = rdata;
		counter++;
	}

	res.timestamp = ts;

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// ID
	counter = 0;
	while (counter < sizeof(int)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		employee.bytes[counter] = rdata;
		counter++;
	}

	res.employeeID = employee;

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// Distance
	counter = 0;
	while (counter < sizeof(float)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		distance.bytes[counter] = rdata;
		counter++;
	}  

	res.distance = distance;

	return res;
}

void Memory::writeTraceRecord(TraceRecord* tr) {
	Serial.println(_currentAddress);

	// Write the TraceRecord identifier
	writeByte((byte)TRACE_RECORD_ID);

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the timestamp
	for (byte i = 0; i < sizeof(unsigned long); i++) {
		writeByte(tr->timestamp.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the employee id
	for (byte i = 0; i < sizeof(int); i++) {
		writeByte(tr->employeeID.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the distance
	for (byte i = 0; i < sizeof(float); i++) {
		writeByte(tr->distance.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write Garbage
	writeByte((byte)GARBAGE_RECORD_ID);

	// We want the next entry to overwrite the 0x03 garbage
	_currentAddress--;

	//_currentAddress += TRACE_RECORD_LEN;
}

/*
*  Reading and Writing GPSRecords to memory
*
*/

GPSRecord Memory::readGPSRecord(long startAddress) {
	float_t latitude;
	float_t longitude;
	timestamp_t ts;
	GPSRecord res;

	byte counter = 0;

	Wire.beginTransmission(_deviceAddress);
	Wire.write((int)(startAddress >> 8)); // MSB
	Wire.write((int)(startAddress & 0xFF)); // LSB 
	Wire.endTransmission();

	// READ THE TYPE
	Wire.requestFrom(_deviceAddress, 1);
	byte rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	if (rdata != GPS_RECORD_ID) {
		return res;
	}

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// Timestamp
	while (counter < sizeof(unsigned long)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		ts.bytes[counter] = rdata;
		counter++;
	}

	res.timestamp = ts;

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// Latitude
	counter = 0;
	while (counter < sizeof(float)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		latitude.bytes[counter] = rdata;
		counter++;
	}

	res.latitude = latitude;

	// Separator
	Wire.requestFrom(_deviceAddress, 1);
	rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	// Longitude
	counter = 0;
	while (counter < sizeof(float)) {
		Wire.requestFrom(_deviceAddress, 1);
		rdata = 0xFF;
		if (Wire.available()) {
			rdata = Wire.read();
		}
		longitude.bytes[counter] = rdata;
		counter++;
	}

	res.longitude = longitude;

	return res;
}

void Memory::writeGPSRecord(GPSRecord* gr) {
	Serial.println(_currentAddress);

	// Write the TraceRecord identifier
	writeByte((byte)GPS_RECORD_ID);

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the timestamp
	for (byte i = 0; i < sizeof(unsigned long); i++) {
		writeByte(gr->timestamp.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the latitude
	for (byte i = 0; i < sizeof(float); i++) {
		writeByte(gr->latitude.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write the longitude
	for (byte i = 0; i < sizeof(float); i++) {
		writeByte(gr->longitude.bytes[i]);
	}

	// Write the separator
	writeByte((byte)MEM_SEPARATOR);

	// Write Garbage
	writeByte((byte)GARBAGE_RECORD_ID);

	// We want the next entry to overwrite the 0x03 garbage
	_currentAddress--;

}


/*
*	Read next record from start address
*/
long Memory::readNextByteRecord(long startAddress, uint8_t* byteArr) {
	byte counter = 0;

	Wire.beginTransmission(_deviceAddress);
	Wire.write((int)(startAddress >> 8)); // MSB
	Wire.write((int)(startAddress & 0xFF)); // LSB 
	Wire.endTransmission();

	Wire.requestFrom(_deviceAddress, 1);
	byte rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	if (rdata == GPS_RECORD_ID) {
		counter = 0;
		while (counter < GPS_RECORD_LEN) {
			Wire.requestFrom(_deviceAddress, 1);
			rdata = 0xFF;
			if (Wire.available()) {
				rdata = Wire.read();
			}
			byteArr[counter] = rdata;
			counter++;
		}
		return startAddress + GPS_RECORD_LEN;
	}

	if (rdata == TRACE_RECORD_ID) {
		counter = 0;
		while (counter < TRACE_RECORD_LEN) {
			Wire.requestFrom(_deviceAddress, 1);
			rdata = 0xFF;
			if (Wire.available()) {
				rdata = Wire.read();
			}
			byteArr[counter] = rdata;
			counter++;
		}
		return startAddress + TRACE_RECORD_LEN;
	}

	return startAddress;

}

/*
*	Debug Read Memory for Records
*/
#if DEBUG_MEMORY
long Memory::debugReadMemory(long startAddress) {
	Wire.beginTransmission(_deviceAddress);
	Wire.write((int)(startAddress >> 8)); // MSB
	Wire.write((int)(startAddress & 0xFF)); // LSB 
	Wire.endTransmission();

	Wire.requestFrom(_deviceAddress, 1);
	byte rdata = 0xFF;
	if (Wire.available()) {
		rdata = Wire.read();
	}

	Serial.println(rdata);

	if (rdata == GPS_RECORD_ID) {
		Serial.println(F("Found GPS Record"));
		GPSRecord gps = readGPSRecord(startAddress);

		Serial.println(gps.timestamp.timestamp);
		Serial.println(gps.latitude.number, 6);
		Serial.println(gps.longitude.number, 6);

		return startAddress + GPS_RECORD_LEN;
	}

	if (rdata == TRACE_RECORD_ID) {
		Serial.println(F("Found Trace Record"));
		TraceRecord tr = readTraceRecord(startAddress);

		Serial.println(tr.timestamp.timestamp);
		Serial.println(tr.distance.number);
		Serial.println(tr.employeeID.eID);

		return startAddress + TRACE_RECORD_LEN;
	}

	return startAddress;

}
#endif

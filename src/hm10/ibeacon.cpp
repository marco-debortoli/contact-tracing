#include "ibeacon.h"


void BtScanInterpreter::initialize() {
	inMessage = false;
	complete = false;
	numberColons = 0;
	writePos = 0;
	uuid[0] = '\0';
	part2[0] = '\0';
	rssi[0] = '\0';    
}

bool BtScanInterpreter::accept(char ch) {
	// Rolling buffer
	memcpy(buf + 0, buf + 1, 4);

	// Set the new character
	buf[4] = ch;

	// Check for the starting message
	if (strncmp(buf, "DISC:", 5) == 0) {
		inMessage = true;
		goodMessage = true;
		numberColons = 1;

		// Refresh new message
		writePos = 0;
		uuid[0] = '\0';
		part2[0] = '\0';
		rssi[0] = '\0';
	} 

	else if (strncmp(buf, "DISCE", 5) == 0) {
		complete = true;
		return false;
	}

	// Check for the end of message
	else if (strncmp(buf + 2, "OK+", 3) == 0) {
		// Only accept messages for valid ibeacons
		if (numberColons != 5) {
			goodMessage = false;
		} else {
			// RSSI needs to be reset
			rssi[writePos-2] = '\0';
		}

		if (goodMessage) {
			char* ret = strstr(uuid, UUID);

			if (ret != NULL) {
				return true;
			}
		}

		inMessage = false;
	}

	// Just wait until everything is good again
	else if (!goodMessage) {
		// ignore this chartacter
	}

	// The separator is a colon
	else if (ch == ':') {

		// We are in part 2 which is the UUID
		if (numberColons == 2) {
			uuid[writePos] = '\0';
		}

		// This is the major, minor and measured power
		else if (numberColons == 3) {
			part2[writePos] = '\0';
		}

		// This is the RSSI
		else if (numberColons == 5) {
			rssi[writePos] = '\0';
		}

		numberColons++;
		writePos = 0;
	}

	else if (numberColons == 2) {
		if (writePos >= sizeof(uuid)) {
			goodMessage = false;
		} else {
			uuid[writePos++] = ch;
		}
	}

	else if (numberColons == 3) {
		if (writePos >= sizeof(part2)) {
			goodMessage = false;
		} else {
			part2[writePos++] = ch;
		}
	}

	else if (numberColons == 5) {
		if (writePos >= sizeof(rssi)) {
			goodMessage = false;
		} else {
			rssi[writePos++] = ch;
		}
	}

	return false;
}

// BLE DEVICE

BLEDevice::BLEDevice(SoftwareSerial* ss) : led(2) {
	_serial = ss;
	_serial->end();
}

void BLEDevice::setMinorNum(char minor[]) {
	beaconChar[0] = minor[0];
	beaconChar[1] = minor[1];
	beaconChar[2] = minor[2];
	beaconChar[3] = minor[3];

	minor[4] = '\0';
	beaconMinor = (int)strtol(minor, NULL, 16);
}

void BLEDevice::setupIScanner() {
  led.mediumBlink();
  sendATCommand("AT+RENEW", 8);
  sendATCommand("AT+IMME1", 8);
  sendATCommand("AT+PWRM1", 8);
  sendATCommand("AT+RESET", 8);
  sendATCommand("AT+ROLE1", 8);
  sendATCommand("AT+RESET", 8);
}

void BLEDevice::sendATCommand(char* cmd, byte resLen) {
  _serial->begin(9600);

  led.quickBlink();

  #if DEBUG_SERIAL
  	Serial.println( cmd );
  #endif

  _serial->write( cmd );
  delay(500);

  byte rx = 0;

  #if DEBUG_SERIAL
  	Serial.println(F("Waiting for AT response"));
  #endif

  while( rx < resLen ) {
    if ( _serial->available() ) {
      rx += 1;
    }
  }

  _serial->end();

  #if DEBUG_SERIAL
	Serial.println(F("DONE"));
  #endif

}

void BLEDevice::bleScan() {
  //Serial.println(F("Starting BLE Scan"));

  led.longBlink();

  // Send the scan request
  _serial->begin(9600);
  _serial->write( "AT+DISI?" );
  delay(1000);

  // Perform the actual scan
  bool continueScan = true;

  BtScanInterpreter interpreter;

  #if DEBUG_SERIAL
  	Serial.println(F("Starting to read scan"));
  #endif

  interpreter.initialize();
  
  while (continueScan) {
    if (_serial->available()) {
      char newChar = _serial->read();
      bool found = interpreter.accept(newChar);

      if (found == true) {
        continueScan = convertBeaconResult(interpreter.part2, interpreter.rssi);
        continueScan = false;
      }

      if (interpreter.complete == true) {
        continueScan = false;
      }
      
    }
  }

  _serial->end();

  #if DEBUG_SERIAL
  	Serial.println(F("Finished Scan"));
  #endif
}


void BLEDevice::setupIBeacon() {
	led.mediumBlink();
	sendATCommand("AT+RENEW", 8);
	sendATCommand("AT+RESET", 8);
	sendATCommand("AT", 2);

	sendATCommand("AT+MARJ0x0001", 13);

	char atMinor[] = "AT+MINO0x0000";

	atMinor[9] = beaconChar[0];
	atMinor[10] = beaconChar[1];
	atMinor[11] = beaconChar[2];
	atMinor[12] = beaconChar[3];

	sendATCommand(atMinor, 13);

	sendATCommand("AT+ADVI5", 8);
	sendATCommand("AT+ADTY3", 8);
	sendATCommand("AT+IBEA1", 8);
	sendATCommand("AT+DELO2", 8);
	sendATCommand("AT+PWRM1", 8);
	sendATCommand("AT+RESET", 8);

	//Serial.println(F("IBeacon Ready"));
}

TraceRecord BLEDevice::createTraceRecord(int minor, float distance) {
	float_t dist;
	dist.number = distance;

	timestamp_t ts;
	ts.timestamp = millis();

	employee_id_t id;
	id.eID = minor;

	TraceRecord tr;
	tr.distance = dist;
	tr.employeeID = id;
	tr.timestamp = ts;

	return tr;
}

bool BLEDevice::convertBeaconResult(char* data, char* rssi) {
  char buf[5];
  int minor;
  int power;
  int rssiPower;

  // MINOR NUMBER - This is the employee number
  for (byte i = 4; i < 8; i++) {
    buf[i-4] = data[i];
  }
  buf[4] = '\0';

  minor = (int)strtol(buf, NULL, 16);

  // Check that the minor number is not the minor number of our iBeacon
  if ( minor == beaconMinor ) {
  	return true;
  }

  // Check that the minor number is not already in the discovered cache
  for (byte i = 0; i < DISCOVERY_CACHE; i++) {
  	if ( discovery[i] == minor ) {
  		return true;
  	}
  }

  // MEASURED POWER - This is the power at 1m
  for (byte i = 8; i < 10; i++) {
    buf[i-8] = data[i];
  }
  buf[2] = '\0';

  power = (int)strtol(buf, NULL, 16);

  // RSSI
  for (byte i = 1; i < 4; i++) {
    buf[i-1] = rssi[i];
  }

  buf[3] = '\0';
  
  rssiPower = (int)strtol(buf, NULL, 10);

  // Calculate the distance
  float distance = (float) pow(10, (((-1*power)-(-1*rssiPower))/(10.0*2)));

  if (distance < 2.0) {
  	#if DEBUG_SERIAL
    	Serial.println(F("Saving result"));
   		Serial.println(minor);
    	Serial.println(distance);
    #endif

    discovery[discoveryIdx] = minor;
    discoveryIdx++;

    if (discoveryIdx > DISCOVERY_CACHE-1) {
    	discoveryIdx = 0;
    }

    TraceRecord tr = createTraceRecord(minor, distance);
    led.mediumBlink();
    led.quickBlink();
    led.mediumBlink();
    processBeaconResult(&tr);

  	return false;

  }

  return true;
  
}

void BLEDevice::processBeaconResult(TraceRecord* tr) {
	#if DEBUG_SERIAL
		Serial.println(F("Processing Beacon Result"));
		Serial.println(tr->distance.number);
	#endif
}
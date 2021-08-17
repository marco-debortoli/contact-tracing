#include "gps.h"

// Global variables for NMEA

GPSModule::GPSModule() : led(12) {}

/*
*  Initialize the GPSModule and make sure it is detected
*  Public
*/
bool GPSModule::begin() {

	if (myGPS.begin() == false) {
	  // Serial.println(F("Ublox GPS not detected at default I2C address."));
	  return false;
	}

	return true;
}

/*
*  Save the GPS values in the respective internal caches
*  Private
*/
void GPSModule::saveGPS(long latitude, long longitude) {
	float convertedLatitude = latitude / 1000000.;
  float convertedLongitude = longitude / 1000000.;

  if (!inLatList(convertedLatitude) && _currentLatIdx != MAX_GPS_COUNT) {  
    // Serial.print("Latitude (deg): ");
    // Serial.println(convertedLatitude, 6);
    
    _latitudes[_currentLatIdx] = convertedLatitude;
    _currentLatIdx++;
  }

  if (!inLonList(convertedLongitude) && _currentLonIdx != MAX_GPS_COUNT) {
    // Serial.print("Longitude (deg): ");
    // Serial.println(convertedLongitude, 6);
    
    _longitudes[_currentLonIdx] = convertedLongitude;
    _currentLonIdx += 1;
  }
}


/*
*  Get the GPS values and return a GPS Record
*  Public
*/
GPSRecord GPSModule::getGPS(MicroNMEA* nmea) {
 //See if new data is available. Process bytes as they come in.

  bool scanning = true;

  led.on();

  while(scanning) {
    myGPS.checkUblox();
    if (nmea->isValid() == true) {
      long latitude_mdeg = nmea->getLatitude();
      long longitude_mdeg = nmea->getLongitude();

      if (_currentLatIdx != MAX_GPS_COUNT || _currentLonIdx != MAX_GPS_COUNT) {
        saveGPS(latitude_mdeg, longitude_mdeg);
      } else {
        float avgLat = averageLat();
        float avgLon = averageLon();

        Serial.print("Final Latitude (deg): ");
        Serial.println(avgLat, 6);
        Serial.print("Final Longitude (deg): ");
        Serial.println(avgLon, 6);

        float_t finalLat;
        finalLat.number = avgLat;

        float_t finalLon;
        finalLon.number = avgLon;

        timestamp_t times;
        times.timestamp = millis();

        GPSRecord gpsr;
        gpsr.latitude = finalLat;
        gpsr.longitude = finalLon;
        gpsr.timestamp = times;

        scanning = false;

        _currentLatIdx = 0;
        _currentLonIdx = 0;

        led.off();
        
        return gpsr;
      }
    } else {
      Serial.print("No Fix - ");
      Serial.print("Num. satellites: ");
      Serial.println(nmea->getNumSatellites());
    }

    delay(250);
  }
}

/*
*  Check if the provided float value is in the cached latitude list
*  Private
*/
bool GPSModule::inLatList(float value) {
  for (int i = 0; i < MAX_GPS_COUNT; i++) {
    if (value == _latitudes[i]) {
      return true;
    }
  }
  return false;
}

/*
*  Check if the provided float value is in the cached longitude list
*  Private
*/
bool GPSModule::inLonList(float value) {
  for (int i = 0; i < MAX_GPS_COUNT; i++) {
    if (value == _longitudes[i]) {
      return true;
    }
  }
  return false;
}

/*
*  Return the average of all the values in the cached latitude list
*  Private
*/
float GPSModule::averageLat() {
  float sum = 0;
  for (int i = 0; i < MAX_GPS_COUNT; i++) {
    sum += _latitudes[i];
  }

  return sum / MAX_GPS_COUNT;
}

/*
*  Return the average of all the values in the cached longitude list
*  Private
*/
float GPSModule::averageLon() {
  float sum = 0;
  for (int i = 0; i < MAX_GPS_COUNT; i++) {
    sum += _longitudes[i];
  }

  return sum / MAX_GPS_COUNT;
}







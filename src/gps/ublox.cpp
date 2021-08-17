#include "ublox.h"

UBLOX_GPS::UBLOX_GPS(void) {}

//Initialize the Serial port
boolean UBLOX_GPS::begin(TwoWire &wirePort, uint8_t deviceAddress)
{
  commType = COMM_TYPE_I2C;
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  _gpsI2Caddress = deviceAddress; //Store the I2C address from user

  return (isConnected());
}

//Want to see the NMEA messages on the Serial port? Here's how
void UBLOX_GPS::setNMEAOutputPort(Stream &nmeaOutputPort)
{
  _nmeaOutputPort = &nmeaOutputPort; //Store the port from user
}

//Called regularly to check for available bytes on the user' specified port
boolean UBLOX_GPS::checkUblox()
{
  if (commType == COMM_TYPE_I2C) {
    return (checkUbloxI2C());
  }

  return false;
}

//Polls I2C for data, passing any new bytes to process()
//Returns true if new bytes are available
boolean UBLOX_GPS::checkUbloxI2C()
{
  if (millis() - lastCheck >= i2cPollingWait)
  {
    //Get the number of bytes available from the module
    uint16_t bytesAvailable = 0;
    _i2cPort->beginTransmission(_gpsI2Caddress);
    _i2cPort->write(0xFD);                     //0xFD (MSB) and 0xFE (LSB) are the registers that contain number of bytes available
    if (_i2cPort->endTransmission(false) != 0) //Send a restart command. Do not release bus.
      return (false);                          //Sensor did not ACK

    _i2cPort->requestFrom((uint8_t)_gpsI2Caddress, (uint8_t)2);
    if (_i2cPort->available()) {
      uint8_t msb = _i2cPort->read();
      uint8_t lsb = _i2cPort->read();
      if (lsb == 0xFF) {
        lastCheck = millis(); //Put off checking to avoid I2C bus traffic
        return (false);
      }
      bytesAvailable = (uint16_t)msb << 8 | lsb;
    }

    if (bytesAvailable == 0) {
      lastCheck = millis(); //Put off checking to avoid I2C bus traffic
      return (false);
    }

    if (bytesAvailable & ((uint16_t)1 << 15)) {
      //Clear the MSbit
      bytesAvailable &= ~((uint16_t)1 << 15);
    }

    while (bytesAvailable)
    {
      _i2cPort->beginTransmission(_gpsI2Caddress);
      _i2cPort->write(0xFF);                     //0xFF is the register to read data from
      if (_i2cPort->endTransmission(false) != 0) //Send a restart command. Do not release bus.
        return (false);                          //Sensor did not ACK

      //Limit to 32 bytes or whatever the buffer limit is for given platform
      uint16_t bytesToRead = bytesAvailable;
      if (bytesToRead > I2C_BUFFER_LENGTH) {
        bytesToRead = I2C_BUFFER_LENGTH;
      }

    TRY_AGAIN:

      _i2cPort->requestFrom((uint8_t)_gpsI2Caddress, (uint8_t)bytesToRead);

      if (_i2cPort->available()) {
        for (uint16_t x = 0; x < bytesToRead; x++) {
          uint8_t incoming = _i2cPort->read(); //Grab the actual character

          //Check to see if the first read is 0x7F. If it is, the module is not ready
          //to respond. Stop, wait, and try again
          if (x == 0) {
            if (incoming == 0x7F) {

              delay(5); //In logic analyzation, the module starting responding after 1.48ms

              goto TRY_AGAIN;
            }
          }

          process(incoming); //Process this valid character
        }
      }
      else
        return (false); //Sensor did not respond

      bytesAvailable -= bytesToRead;
    }
  }

  return (true);

} //end checkUbloxI2C()


//Processes NMEA and UBX binary sentences one byte at a time
//Take a given byte and file it into the proper array
void UBLOX_GPS::process(uint8_t incoming)
{
  if ((currentSentence == NONE) || (currentSentence == NMEA))
  {
    if (incoming == '$')
    {
      currentSentence = NMEA;
    }
    else
    {
      //This character is unknown or we missed the previous start of a sentence
    }
  }

  if (currentSentence == NMEA)
  {
    processNMEA(incoming); //Process each NMEA character
  }
}

//Returns true if I2C device ack's
boolean UBLOX_GPS::isConnected(uint16_t maxWait)
{
  if (commType == COMM_TYPE_I2C)
  {
    _i2cPort->beginTransmission((uint8_t)_gpsI2Caddress);
    if (_i2cPort->endTransmission() != 0)
      return false; //Sensor did not ack
  }

  return true;
}

//This is the default or generic NMEA processor. We're only going to pipe the data to serial port so we can see it.
//User could overwrite this function to pipe characters to nmea.process(c) of tinyGPS or MicroNMEA
//Or user could pipe each character to a buffer, radio, etc.
void UBLOX_GPS::processNMEA(char incoming)
{
  //If user has assigned an output port then pipe the characters there
  if (_nmeaOutputPort != NULL)
    _nmeaOutputPort->write(incoming); //Echo this byte to the serial port
}

#include "pn532_nfc.h"

NFCModule::NFCModule(SNEP* snep) {
	_snep = snep;
}

// convert an 8-bit byte to a string of 2 hexadecimal characters
void NFCModule::byte_to_str(char* buff, uint8_t val) {
  buff[0] = nibble_to_hex(val >> 4);
  buff[1] = nibble_to_hex(val);
}

// convert a 4-bit nibble to a hexadecimal character
char NFCModule::nibble_to_hex(uint8_t nibble) {
  nibble &= 0xF;
  return nibble > 9 ? nibble - 10 + 'A' : nibble + '0';
}


bool NFCModule::receiveEmployeeID(char employeeID[]) {
    Serial.println(F("Receiving ID"));
    
    resetBuf();
    int16_t len = _snep->read(_nfcBuf, NFC_BUF_LEN, 3000);

    if (len > 0) {
      Serial.println(F("get a SNEP message:"));

      byte_to_str(&employeeID[0], _nfcBuf[3]);
      byte_to_str(&employeeID[2], _nfcBuf[4]);
      employeeID[4] = '\0';
      
      return true;
    }

    return false;
}


void NFCModule::resetBuf() {
	for (byte i = 0; i < NFC_BUF_LEN; i++) {
		_nfcBuf[i] = 0;
	}
}


bool NFCModule::transferData(Memory* memory) {
    byte ret = 0;
    Serial.println(F("Attempting Write"));

    long addr = 0;
    long nextAddr = 0;
    
    bool dataWrite = false;

    do {
    	resetBuf();
    	addr = nextAddr;
    	nextAddr = memory->readNextByteRecord(addr, &_nfcBuf[3]);
 
    	if (nextAddr != addr) {
    		_nfcBuf[0] = 0xD5;
	    	_nfcBuf[1] = 0x00;
	    	_nfcBuf[2] = (uint8_t)nextAddr-addr;

    		ret = _snep->write(_nfcBuf, (uint8_t)nextAddr-addr, 1000);

		    if (ret != 1) {
		      return false;
		    }

		    dataWrite = true;
    	}

    } while (addr != nextAddr);

    if (dataWrite) {
    	resetBuf();
	    _nfcBuf[0] = 0xD0;
	    _nfcBuf[1] = 0x00;
	    _nfcBuf[2] = 0x00;
	    ret = _snep->write(_nfcBuf, 3, 1000);

	    if (ret != 1) {
	      return false;
	    }

	    Serial.println(F("Write Complete"));
    	return true;
    }

    return false;
    
}
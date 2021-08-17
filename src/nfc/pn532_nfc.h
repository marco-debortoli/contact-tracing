#ifndef CONTACT_TRACING_NFC_H
#define CONTACT_TRACING_NFC_H

#include <Arduino.h>
#include "libs/snep.h"
#include "../memory/memory.h"

#define NFC_BUF_LEN 20

class NFCModule {

public:
	NFCModule(SNEP* snep);

	bool receiveEmployeeID(char employeeID[]);
	bool transferData(Memory* memory);

private:
	SNEP* _snep;

	char nibble_to_hex(uint8_t nibble);
	void byte_to_str(char* buff, uint8_t val);
	void resetBuf();

	uint8_t _nfcBuf[NFC_BUF_LEN];



};

#endif
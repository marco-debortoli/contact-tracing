#include "snep.h"


int8_t SNEP::write(const uint8_t *buf, uint8_t len, uint16_t timeout) {
	if (0 >= llcp.activate(timeout)) {
		Serial.println(F("Failed to setup PN532 as target"));
		return -1;
	}

	if (0 >= llcp.connect(timeout)) {
		Serial.println(F("Failed to get connection"));
		return -2;
	}

	// response a success SNEP message
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_REQUEST_PUT;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = len;

	if (0 >= llcp.write(headerBuf, 6, buf, len)) {
		return -3;
	}

	uint8_t rbuf[16];
	if (6 > llcp.read(rbuf, sizeof(rbuf))) {
		return -4;
	}

	// expect a put request
	if (SNEP_RESPONSE_SUCCESS != rbuf[1]) {
		Serial.println(F("Expect a success response"));
		return -4;
	}

	llcp.disconnect(timeout);

	return 1;
}

int16_t SNEP::read(uint8_t *buf, uint8_t len, uint16_t timeout) {
	if (0 >= llcp.activate(timeout)) {
		Serial.println(F("Failed to setup PN532 as target"));
		return -1;
	}

	if (0 >= llcp.waitForConnection(timeout)) {
		Serial.println(F("Failed to get connection"));
		return -2;
	}

	uint16_t status = llcp.read(buf, len);

	if (6 > status) {
		return -3;
	}

	uint32_t length = (buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5];
	for (uint8_t i = 0; i < length; i++) {
		buf[i] = buf[i + 6];
	}

	// response a success SNEP message
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_RESPONSE_SUCCESS;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = 0;
	llcp.write(headerBuf, 6);

	return length;
}

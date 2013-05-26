#include "twi.h"

TWI::TWI() {
	this->fdAtmega = wiringPiI2CSetup(3);
	if(this->fdAtmega > -1) {
		;
	} else {
		elog << "Błąd inicjalizacji komunikacji TWI!";
	}
}

LngLat TWI::getPosition() {
	float lat = this->getFloat(1);
	float lng = this->getFloat(9);

	return LngLat(lng, lat);
}

LngLat TWI::getCurrentGoal() {
	float lat = this->getFloat(49);
	float lng = this->getFloat(57);

	return LngLat(lng, lat);
}

float TWI::getFloat(int startAddress) {
	union floatOrByte {
		char b[4];
		float f;
	} u;

	for(int i = 0; i<4; i++) {
		u.b[i] = wiringPiI2CReadReg8(this->fdAtmega, startAddress + i);
	}
	return u.f;
}

int TWI::writeFloat(int startAddress, float data) {
	union floatOrByte {
		char b[4];
		float f;
	} u;
	u.f = data;

	int resp, err;
	bool allOK = true;
	for(int i = 0; i<4; i++) {
		resp = wiringPiI2CWriteReg8(this->fdAtmega, startAddress + i, u.b[i]);
		if(resp<=-1) { // error
			allOK = false;
			err = resp;
			//elog << "nie powodzenie wysłania bajtu";
		}
	}

	if(!allOK)
		return err;
	return resp;
}

int TWI::writeFirstGoal(LngLat goal) {
	int success = 0;
	success = this->writeFloat(17, goal.lat);
	if(success<=-1)
		return success;
	success = this->writeFloat(25, goal.lat);
	return success;
}

int TWI::writeNextGoal(LngLat goal) {
	int success = 0;
	success = this->writeFloat(33, goal.lat);
	if(success<=-1)
		return success;
	success = this->writeFloat(41, goal.lat);
	return success;
}
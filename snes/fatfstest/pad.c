#include "data.h";
#include "pad.h";
#include "debug.h";

void enablePad(void) {
	// Enable pad reading and NMI
	*(byte*)0x4200 = 0x01;
}

void disablePad(void) {
    // Enable pad reading and NMI
    *(byte*)0x4200 = 0x00;
}

padStatus readPad(byte padNumber) {
	word test;
	padStatus *status;
	padNumber = padNumber << 1;
	test = (word) *(byte*)0x4218+padNumber << 8;
	test |= (word) *(byte*)0x4219+padNumber;
	status = (padStatus *) &test;
	return *status;
}

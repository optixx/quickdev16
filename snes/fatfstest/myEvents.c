#include "data.h";
#include "pad.h";
#include "event.h";

extern padStatus pad1;
extern word scrollValue;

char fadeOut(word counter) {
	static byte fadeOutValue;

	if(counter == 0) {
		// init fade value
		fadeOutValue = 0x0f;
	} else {
		fadeOutValue--;
	}

	*(byte*) 0x2100 = fadeOutValue;	

	if(fadeOutValue == 0x00) {
		return EVENT_STOP;
	} else { 
		return EVENT_CONTINUE;
	}
}

char fadeIn(word counter) {
	static byte fadeInValue;

	if(counter == 0) {
		// init fade value
		fadeInValue = 0x00;
	} else {
		fadeInValue++;
	}

	*(byte*) 0x2100 = fadeInValue;	

	if(fadeInValue >= 0x0f) {
		return EVENT_STOP;
	} else { 
		return EVENT_CONTINUE;
	}
}

char mosaicOut(word counter) {
	static byte mosaicOutValue;

	if(counter == 0) {
		// init fade value
		mosaicOutValue = 0xff;
	} else {
		mosaicOutValue -= 0x10;
	}

	*(byte*) 0x2106 = mosaicOutValue;	

	if(mosaicOutValue == 0x0f) {
		return EVENT_STOP;
	} else { 
		return EVENT_CONTINUE;
	}
}

char mosaicIn(word counter) {
	static byte mosaicInValue;

	if(counter == 0) {
		// init fade value
		mosaicInValue = 0x0f;
	} else {
		mosaicInValue += 0x10;
	}

	*(byte*) 0x2106 = mosaicInValue;	

	if(mosaicInValue == 0xff) {
		return EVENT_STOP;
	} else { 
		return EVENT_CONTINUE;
	}
}

char NMIReadPad(word counter) {
	pad1 = readPad((byte) 0);

	return EVENT_CONTINUE;
}

char scrollLeft(word counter) {
	scrollValue++;

	*(byte*) 0x210d = (byte) scrollValue;
	*(byte*) 0x210d = (byte) (scrollValue>>8);

	return EVENT_CONTINUE;
}
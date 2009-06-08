#include <stdlib.h>

#include "data.h";
#include "event.h";

event *events;

void initEvents(void) {
	events = NULL;
}

event *createEvent(char (*callback)(word counter)) {
	event *myEvent;

	myEvent = (event*) malloc(sizeof(event));

	myEvent->VBlankCount = 0;
	myEvent->callback = callback;
	myEvent->nextEvent = NULL;
	myEvent->previousEvent = NULL;


	return myEvent;
}

event* addEvent(char (*callback)(word counter), int noDuplicateCallback) {

	event *lastEvent;
	event *myEvent;

	if(events == NULL) {
		events = createEvent(callback);
		return events;
	} else {
		lastEvent = events;
		// TODO optimise this with noduplicate
		while(lastEvent->nextEvent != NULL) {
			if(noDuplicateCallback == 1 && lastEvent->callback == *callback) {
				return NULL;
			}
			lastEvent = lastEvent->nextEvent;
		}
		if(noDuplicateCallback == 1 && lastEvent->callback == *callback) {
			return NULL;
		}
		myEvent = createEvent(callback);
		myEvent->previousEvent = lastEvent;
		lastEvent->nextEvent = myEvent;
		return myEvent;
	}


}

void removeEvent(event *eventElement) {

	byte alone = 0;
	event *next, *previous;

	next = eventElement->nextEvent;
	previous = eventElement->previousEvent;

	if(eventElement->nextEvent != NULL && eventElement->previousEvent != NULL) {
		alone++;
		next->previousEvent = previous;
		previous->nextEvent = next;

	} else if(eventElement->nextEvent != NULL) {
		alone++;
		next->previousEvent = NULL;
		events = next;

	} else if(eventElement->previousEvent != NULL) {
		alone++;
		previous->nextEvent = NULL;
	}

	free(eventElement);

	if(alone == 0) {
		events = NULL;
	}
}

void processEvents(void) {

	event *currentEvent;
	char returnValue;

	currentEvent = events;
	while(currentEvent != NULL) {
		returnValue = currentEvent->callback(currentEvent->VBlankCount);
		if(returnValue == EVENT_CONTINUE) {
			currentEvent->VBlankCount++;
		} else {
			removeEvent(currentEvent);
		}
		currentEvent = currentEvent->nextEvent;
	}

}

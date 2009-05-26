typedef struct event{
	word VBlankCount;
	char (*callback)(word counter);
	struct event *previousEvent;
	struct event *nextEvent;
} event;

#define EVENT_STOP 0
#define EVENT_CONTINUE 1

extern event *events;

void initEvents(void);
extern event* addEvent(char (*callback)(word counter), int noDuplicateCallback);
extern void removeEvent(event *eventElement);
extern void processEvents(void);

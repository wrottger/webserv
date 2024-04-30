#ifndef EVENTSDATA_HPP
#define EVENTSDATA_HPP

enum EventType {
	LISTENING,
	CLIENT,
	CGI
};

struct EventsData {
	int fd;
	EventType eventType;
	void *objectPointer;
};

#endif
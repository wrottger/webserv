#ifndef EVENTSDATA_HPP
#define EVENTSDATA_HPP
#include <sys/epoll.h>

enum EventType {
	LISTENING,
	CLIENT,
	CGI
};

struct EventsData {
	int fd;
	uint32_t eventMask;
	EventType eventType;
	void *objectPointer;
};

#endif
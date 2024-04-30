#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include "EventsData.hpp"
#include "HttpError.hpp"
#include "HttpHeader.hpp"
#include "Logger.hpp"
#include "SocketHandling.hpp"
#include "Cgi.hpp"
#include <sys/epoll.h>
#include <ctime>
#include <iostream>
#include <list>
#include <new>

#define MAX_EVENTS 64
#define EPOLL_TIMEOUT 300
#define BUFFER_SIZE 8192
#define CLIENT_TIMEOUT 3 // Seconds

class Client;

class EventHandler {

public:
	EventHandler(SocketHandling &sockets);
	~EventHandler();
	void start();
	EventsData *createNewEvent(int fd, EventType type, Client *client);
	void addEventToList(EventsData *eventData);
	int getEpollFd() const;

private:
	int _epollFd;
	std::vector<int> _listeningSockets;
	std::list<EventsData *> _eventDataList;

	EventHandler();
	EventHandler(EventHandler const &other);
	EventHandler &operator=(EventHandler const &other);

	bool isListeningSocketTriggered(epoll_event events_arr[], int n) const;
	void handleTimeouts();
	void processCleanUpList(std::list<EventsData *> &cleanUpList);
	void destroyClient(Client *client);
	void acceptNewClient(EventsData *eventData);
	void readFromClient(EventsData &eventData, std::list<EventsData *> &cleanUpList);
};

#endif
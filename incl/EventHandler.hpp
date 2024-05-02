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
	int registerEvent(int fd, EventType type, Client *client);
	void unregisterEvent(int fd);
	void unregisterEvent(EventsData *eventData);
	void addToCleanUpList(int fd);
	void addToCleanUpList(EventsData *eventData);

private:
	int _epollFd;
	std::vector<int> _listeningSockets;
	std::list<EventsData *> _eventDataList;
	std::list<EventsData *> _cleanUpList;

	EventHandler();
	EventHandler(EventHandler const &other);
	EventHandler &operator=(EventHandler const &other);

	void handleClientTimeouts();
	void processCleanUpList();
	void acceptNewClient(EventsData *eventData);
	void readFromClient(EventsData &eventData);
};

#endif
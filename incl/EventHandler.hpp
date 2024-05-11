#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include "Cgi.hpp"
#include "Client.hpp"
#include "EventsData.hpp"
#include "HttpError.hpp"
#include "HttpHeader.hpp"
#include "Logger.hpp"
#include "SocketHandling.hpp"
#include <sys/epoll.h>
#include <ctime>
#include <iostream>
#include <list>
#include <new>

#define MAX_EVENTS 64

class Client;

class EventHandler {
public:
	static EventHandler &getInstance();
	void start();
	EventsData *registerEvent(int fd, EventType type, Client *client);

private:
	static EventHandler _instance;
	int _epollFd;
	bool isRunning;
	EventsData *_currentEvent;
	std::vector<int> _listeningSockets;
	std::list<EventsData *> _eventDataList;
	std::list<EventsData *> _cleanUpList;

	EventHandler();
	~EventHandler();
	EventHandler(EventHandler const &other);
	EventHandler &operator=(EventHandler const &other);

	EventsData *createNewEvent(int fd, EventType type, Client *client);
	// int getEpollFd() const;
	void unregisterEvent(int fd);
	void unregisterEvent(EventsData *eventData);
	void addToCleanUpList(int fd);
	void addToCleanUpList(EventsData *eventData);
	void removeInactiveClients();
	void processCleanUpList();
	void acceptNewClient(EventsData *eventData);
	std::string ft_inet_ntop(int af, const void *src);
};

#endif
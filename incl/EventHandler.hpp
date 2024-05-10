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

class Client;

class EventHandler {

public:
	void start();
	static EventHandler &getInstance();

private:
	static EventHandler _instance;
	int _epollFd;
	bool isRunning;
	std::vector<int> _listeningSockets;
	std::list<EventsData *> _eventDataList;
	std::list<EventsData *> _cleanUpList;

	EventHandler();
	~EventHandler();
	EventHandler(EventHandler const &other);
	EventHandler &operator=(EventHandler const &other);

	EventsData *createNewEvent(int fd, EventType type, Client *client);
	int getEpollFd() const;
	int registerEvent(int fd, EventType type, Client *client);
	void unregisterEvent(int fd);
	void unregisterEvent(EventsData *eventData);
	void addToCleanUpList(int fd);
	void addToCleanUpList(EventsData *eventData);
	void removeInactiveClients();
	void processCleanUpList();
	void acceptNewClient(EventsData *eventData);
	std::string ft_inet_ntop(int af, const void* src);
};

#endif
#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

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
#define EPOLL_TIMEOUT 300
#define BUFFER_SIZE 8192
#define CLIENT_TIMEOUT 3 // Seconds

class EventHandler {
	class Client;

public:
	EventHandler(SocketHandling &sockets);
	~EventHandler();
	void start();

private:
	int _epollFd;
	std::vector<int> _listeningSockets;
	std::list<EventHandler::Client *> _clients;

	EventHandler();
	EventHandler(EventHandler const &other);
	EventHandler &operator=(EventHandler const &other);

	bool isListeningSocketTriggered(epoll_event events_arr[], int n) const;
	void handleTimeouts();
	void handleToCloseConnections(std::list<int> &cleanUpList);
	void destroyClient(EventHandler::Client *client);
	void acceptNewClient(epoll_event events_arr[], int n);
	bool readFromClient(epoll_event events[], int n, std::list<int> &cleanUpList);
	Client *findClient(int fd);
};

class EventHandler::Client {
public:
	Client(int fd);
	~Client();
	int getFd();
	std::time_t getLastModified();
	void updateTime();
	bool isHeaderComplete();
	bool isBodyComplete();
	void parseBuffer(const char *buffer);

private:
	Client();
	Client(Client const &other);
	Client &operator=(Client const &other);
	HttpHeader *_requestObject;
	int _fd;
	std::time_t _lastModified;
};

#endif
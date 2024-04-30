#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HttpHeader.hpp"
#include <ctime>
#include <unistd.h>

class EventHandler;

class Client {
public:
	Client(int fd, EventHandler *eventHandler);
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
	EventHandler *_eventHandler;
};

#endif
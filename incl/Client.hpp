#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HttpHeader.hpp"
#include <unistd.h>
#include <ctime>

class EventHandler;

class Client {
public:
	Client(int fd, EventHandler *eventHandler);
	~Client();
	int getFd() const;
	std::time_t getLastModified() const;
	void updateTime();
	bool isHeaderComplete() const;
	bool isBodyComplete() const;
	void parseBuffer(const char *buffer);
	EventHandler *getEventHandler() const;
	HttpHeader *getHeaderObject() const;

private:
	Client();
	Client(Client const &other);
	Client &operator=(Client const &other);
	HttpHeader *_headerObject;
	int _fd;
	std::time_t _lastModified;
	EventHandler *_eventHandler;
};

#endif
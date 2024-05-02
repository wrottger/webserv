#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HttpHeader.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <ctime>

class EventHandler;

#define BUFFER_SIZE 8192
#define CLIENT_TIMEOUT 3

class Client {
public:
	Client(int fd);
	~Client();
	void process(uint32_t events);
	bool canBeDeleted() const;
	bool isTimeouted() const;

private:
	Client();
	Client(Client const &other);
	Client &operator=(Client const &other);

	int getFd() const;
	void updateTime();
	bool isHeaderComplete() const;
	void parseBuffer(const char *buffer);
	std::time_t getLastModified() const;
	HttpHeader *getHeaderObject() const;
	void readFromClient();

private:
	HttpHeader *_headerObject;
	std::time_t _lastModified;
	int _fd;
	bool _canBeDeleted;
};

#endif
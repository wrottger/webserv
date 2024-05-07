#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/epoll.h>
#include <unistd.h>
#include <ctime>
#include "HttpHeader.hpp"
#include "HttpResponse.hpp"

class EventHandler;

#define BUFFER_SIZE 8192
#define CLIENT_TIMEOUT 3

class Client {
public:
	Client(int fd, std::string ip);
	~Client();
	void process(uint32_t events);
	bool canBeDeleted() const;
	bool isTimeouted() const;

private:
	enum State {
		READING_HEADER,
		READING_BODY,
		WAITING_FOR_CGI,
		SENDING_RESPONSE,
		FINISHED
	};

private:
	Client();
	Client(Client const &other);
	Client &operator=(Client const &other);

	int getFd() const;
	void updateTime();
	bool isHeaderComplete() const;
	std::time_t getLastModified() const;
	HttpHeader *getHeaderObject() const;
	void readFromClient();

private:
	HttpHeader *_headerObject;
	std::time_t _lastModified;
	HttpResponse *httpResponse;
	int _fd;
	bool _canBeDeleted;
	State _state;
	std::string _ip;
	std::string _bodyBuffer;
};

#endif
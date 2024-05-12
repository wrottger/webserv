#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HttpHeader.hpp"
#include "HttpResponse.hpp"
#include "EventsData.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <ctime>

class EventHandler;
class Cgi;

#define BUFFER_SIZE 8192
#define CLIENT_TIMEOUT 3

class Client {
public:
	Client(int fd, std::string ip);
	~Client();
	void process(EventsData *eventData);
	bool isDeletable() const;
	bool isTimeouted() const;
	int getFd();
	HttpHeader &getHeaderObject();
	std::string &getBodyBuffer();
	const std::string &getIp() const;
	bool hasCgi() const;
	Cgi *getCgi();

private:
	enum State {
		READING_HEADER,
		CGI_RESPONSE,
		NORMAL_RESPONSE,
		FINISHED
	};

private:
	Client();
	Client(Client const &other);
	Client &operator=(Client const &other);

	void updateTime();
	bool isHeaderComplete() const;
	std::time_t getLastModified() const;
	void readFromClient();

private:
	HttpHeader *_header;
	HttpResponse *_responseHttp;
	Cgi *_responseCgi;
	std::time_t _lastModified;
	int _fd;
	bool _canBeDeleted;
	State _state;
	std::string _ip;
	std::string _bodyBuffer;
};

#endif
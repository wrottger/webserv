#ifndef CGI_HPP
#define CGI_HPP

#include "Config.hpp"
#include "HttpHeader.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <ctime>
#include <iostream>
#include <vector>

class Cgi {
private:
	std::string _outputBuffer;
	int _sockets[2];
	time_t _lastModified;
	bool _isFinished;
	int _errorCode;

private:
	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	const char *createEnviroment();
	void executeCgi(const std::string &bodyBuffer, const int &epollFd, const HttpHeader &requestObject);
	int excecuteChild(const std::string &bodyBuffer, const HttpHeader &requestObject);

public:
	Cgi(const std::string &bodyBuffer, const int &epollFd, const HttpHeader &requestObject);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
};

#endif
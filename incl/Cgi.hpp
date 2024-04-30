#ifndef CGI_HPP
#define CGI_HPP

#include "Client.hpp"
#include "Config.hpp"
#include "EventHandler.hpp"
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
	static const size_t _maxBufferSize = 1024 * 1024;
	size_t _currentBufferSize;
	int _sockets[2];
	time_t _timeCreated;
	bool _isFinished;
	int _errorCode;
	static const time_t _timeout = 5;

private:
	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	char **createEnvironment(const HttpHeader *headerObject);
	char **createArguments();
	void executeCgi(const std::string &bodyBuffer, Client *client);
	int executeChild(const HttpHeader *headerObject);
	

public:
	Cgi(const std::string &bodyBuffer, Client *client);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
};

#endif
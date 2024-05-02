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

#define SEND_SIZE 8192
#define CGI_TIMEOUT 5
#define MAX_CGI_BUFFER_SIZE 1024 * 1024

class Cgi {
private:
	static const size_t _maxBufferSize = MAX_CGI_BUFFER_SIZE;
	std::string _outputBuffer;
	size_t _currentBufferSize;
	int _sockets[2];
	time_t _timeCreated;
	bool _isFinished;
	int _errorCode;
	static const time_t _timeout = CGI_TIMEOUT;
	const std::string _bodyBuffer;
	HttpHeader *_headerObject;

private:
	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	char **createEnvironment(const HttpHeader *headerObject);
	char **createArguments();
	void executeCgi();
	int executeChild(const HttpHeader *headerObject);

public:
	Cgi(const std::string &bodyBuffer, HttpHeader *headerObject);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
};

#endif
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
#include <string>

#define SEND_SIZE 8192
#define CGI_TIMEOUT 5
#define MAX_CGI_BUFFER_SIZE 1024 * 1024

class Cgi {
private:
	enum State {
		READING_BODY,
		CREATE_CGI_PROCESS, // TODO Set flag in client that it should not check for timeout anymore
		WAITING_FOR_CHILD,
		SENDING_RESPONSE,
		FINISHED
	};

	static const size_t _maxBufferSize = MAX_CGI_BUFFER_SIZE;
	std::string _childReturnBuffer;
	size_t _currentBufferSize;
	int _sockets[2];
	time_t _timeCreated;
	bool _isFinished;
	int _errorCode;
	static const time_t _timeout = CGI_TIMEOUT;
	std::string _bodyBuffer;
	HttpHeader *_headerObject;
	char **_enviromentVariables;
	size_t _contentLength;
	std::string _clientIp;
	State _currentState;
	std::string _sendToChildBuffer;
	int	_fd;

private:
	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);

	std::vector<std::string> createEnviromentVariables();
	char **createArguments();
	std::string toString(size_t number);
	std::string toString(int number);
	void executeCgi();
	int executeChild(const HttpHeader *headerObject);
	void readBody();
	int decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody);
	int createCgiProcess();

public:
	Cgi(const std::string &bodyBuffer, HttpHeader *headerObject, std::string clientIp, int fd);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
	void process();
};

#endif
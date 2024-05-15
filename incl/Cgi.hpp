#ifndef CGI_HPP
#define CGI_HPP

#include "Client.hpp"
#include "Config.hpp"
#include "EventHandler.hpp"
#include "EventsData.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <wait.h>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "Utils.hpp"
#include <signal.h>

#define SEND_SIZE 8192
#define CGI_TIMEOUT 1
#define MAX_CGI_BUFFER_SIZE 1024 * 1024

class Cgi {
private:
	enum State {
		CHECK_METHOD,
		READING_BODY,
		CREATE_CGI_PROCESS, // TODO Set flag in client that it should not check for timeout anymore
		READING_FROM_CHILD,
		WAITING_FOR_CHILD,
		SENDING_TO_CHILD,
		SENDING_RESPONSE,
		FINISHED
	};

	Client *_client;
	const HttpHeader &_header;
	std::string &_requestBody;
	std::string _clientIp;
	int _fd;

	static const size_t _maxBufferSize = MAX_CGI_BUFFER_SIZE;
	size_t _contentLength;

	std::string _cgiToServerBuffer;
	size_t _currentCgiToServerBufferSize;

	time_t _timeCreated;
	int _sockets[2];

	static const time_t _timeout = CGI_TIMEOUT;
	int _errorCode;
	State _state;
	std::string _serverToCgiBuffer;
	pid_t _childPid;

	EventsData *_eventData;

private:
	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	Config *_config;
	char **createEnviromentVariables();
	char **createArguments();
	void executeCgi();
	int executeChild();
	int readBody(EventsData *eventData);
	int decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody);
	int createCgiProcess();
	int sendToChild();
	int readFromChild();
	std::string createErrorResponse(int errorCode);
	int checkIfValidMethod();
	int checkIfValidFile();
	bool isTimedOut();

public:
	Cgi(Client *client);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
	void process(EventsData *eventData);
	EventsData *getEventData() const;
};

#endif
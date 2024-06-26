#ifndef CGI_HPP
#define CGI_HPP

#include "CgiResponse.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "EventHandler.hpp"
#include "EventsData.hpp"
#include "HttpChunkedDecoder.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>
#include <cctype>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#define SEND_SIZE 8192
#define CGI_TIMEOUT 30
#define MAX_CGI_BUFFER_SIZE 200000000

class Cgi {
private:
	enum State {
		CHECK_METHOD,
		READING_BODY,
		CREATE_CGI_PROCESS,
		READING_FROM_CHILD,
		WAITING_FOR_CHILD,
		SENDING_TO_CHILD,
		SENDING_RESPONSE,
		FINISHED
	};

	Client *_client;
	HttpChunkedDecoder _chunkedDecoder;
	const HttpHeader &_header;
	std::vector<char> &_requestBody;
	std::string _clientIp;
	int _fd;
	size_t _contentLength;
	std::string _cgiToServerBuffer;
	time_t _timeCreated;
	int _sockets[2];
	int _errorCode;
	State _state;
	std::vector<char> _serverToCgiBuffer;
	pid_t _childPid;
	EventsData *_eventData;
	size_t _bytesSendToCgi;
	Config &_config;
	CgiResponse _cgiResponse;
	bool _isInternalRedirect;
	std::string _InternalRedirectLocation;
	size_t _bodyBytesRead;
	size_t _maxBodySize;

	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	char **createEnviromentVariables();
	char **createArguments();
	int executeChild();
	int readBody(EventsData *eventData);
	int createCgiProcess();
	int sendToChild();
	int readFromChild();
	std::string createErrorResponse(int errorCode);
	int checkIfValidMethod();
	int checkIfValidFile();
	bool isTimedOut();
	std::string generateErrorResponse(const int errorCode);
	std::string getErrorMessage(const int errorCode);


public:
	Cgi(Client *client);
	~Cgi();

	bool isFinished() const;
	void process(EventsData *eventData);
	EventsData *getEventData() const;
	bool isInternalRedirect() const;
	std::string getInternalRedirectLocation() const;
};

#endif
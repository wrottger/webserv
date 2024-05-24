#ifndef CGI_HPP
#define CGI_HPP

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
#define CGI_TIMEOUT 3
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
		FINISHED,
	};

	Client *_client;
	HttpChunkedDecoder _chunkedDecoder;
	const HttpHeader &_header;
	std::vector<char> &_requestBody;
	std::string _clientIp;
	int _fd;
	size_t _contentLength;
	std::string _cgiToServerBuffer;
	size_t _currentCgiToServerBufferSize;
	time_t _timeCreated;
	int _sockets[2];
	int _errorCode;
	State _state;
	std::vector<char> _serverToCgiBuffer;
	pid_t _childPid;
	EventsData *_eventData;
	size_t _bytesSendToCgi;
	std::map<std::string, std::string> _responseHeaders;

	enum decodeState {
		READ_SIZE,
		READ_SIZE_END,
		READ_CHUNK,
		READ_TRAILER_CR,
		READ_TRAILER_LF
	};

	Cgi();
	Cgi(const Cgi &other);
	Cgi &operator=(const Cgi &other);
	Config &_config;
	char **createEnviromentVariables();
	char **createArguments();
	void executeCgi();
	int executeChild();
	int readBody(EventsData *eventData);
	int createCgiProcess();
	int sendToChild();
	int readFromChild();
	std::string createErrorResponse(int errorCode);
	int checkIfValidMethod();
	int checkIfValidFile();
	bool isTimedOut();
	// bool isValidStatusCode(const std::string &statusCode) const;
	// bool isValidStatusLine(const std::string &line) const;
	// bool isValidHeaderField(const std::string &line) const;
	// bool isValidContentType(const std::string &line) const;
	// bool isContentTypeField(const std::string &line) const;
	int checkCgiReturnHeader();
	int addHeaderField(const std::string &line);

public:
	Cgi(Client *client);
	~Cgi();

	bool isFinished() const;
	int getErrorCode() const;
	void process(EventsData *eventData);
	EventsData *getEventData() const;
};

#endif
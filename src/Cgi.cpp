#include "Cgi.hpp"
#include <cstdlib>
#include <cstring>

char *dupString(const std::string &str) {
	char *cstr = new char[str.length() + 1];
	std::strcpy(cstr, str.c_str());
	return cstr;
}

std::string Cgi::toString(size_t number) {
	std::stringstream result;
	result << number;
	return result.str();
}

std::string Cgi::toString(int number) {
	std::stringstream result;
	result << number;
	return result.str();
}

char **Cgi::createEnviromentVariables() {
	std::vector<std::string> envp;

	envp.push_back("AUTH_TYPE=");
	envp.push_back("CONTENT_LENGTH=" + toString(_contentLength));
	if (_header.isInHeader("content-type")) {
		std::string contentType = _header.getHeader("content-type");
		envp.push_back("CONTENT_TYPE=" + contentType);
	}
	envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envp.push_back("PATH_INFO=" + Config::getInstance()->getFilePath(_header.getPath(), "localhost"));
	envp.push_back("PATH_TRANSLATED=");
	envp.push_back("QUERY_STRING=" + _header.getQuery());
	envp.push_back("REMOTE_ADDR=" + _clientIp);
	envp.push_back("REMOTE_HOST=" + _clientIp);
	// envp.push_back("REMOTE_IDENT=");
	envp.push_back("REQUEST_METHOD=" + _header.getMethod());
	envp.push_back("SCRIPT_NAME=" + _header.getPath()); // FIXME: Maybe wrong when: /path/to/script.py/param1/param2
	envp.push_back("SERVER_NAME=" + _header.getHost());
	envp.push_back("SERVER_PORT=" + toString(_header.getPort())); // TODO: Change to actual server port
	envp.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envp.push_back("SERVER_SOFTWARE=WebServ/1.0");

	for (std::map<std::string, std::string>::const_iterator it = _header.getHeaders().begin(); it != _header.getHeaders().end(); ++it) {
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		std::replace(key.begin(), key.end(), '-', '_');
		envp.push_back("HTTP_" + key + "=" + it->second);
	}

	char **result = new char *[envp.size() + 1];
	for (size_t i = 0; i < envp.size(); i++) {
		envp[i] = new char[envp[i].length() + 1];
		std::strcpy(result[i], envp[i].c_str());
	}
	result[envp.size()] = NULL;

	return result;
}

char **Cgi::createArguments() {

	std::string interpreterPath = Config::getInstance()->getCgiInterpreterPath(_header.getPath(), _header.getHost());
	if (interpreterPath.empty()) {
		perror("Failed to get interpreter path");
		exit(255);
	}
	std::string scriptPath = Config::getInstance()->getCgiScriptPath(_header.getPath(), _header.getHost());
	if (scriptPath.empty()) {
		perror("Failed to get script path");
		exit(255);
	}
	char **argv = new char *[3];
	argv[0] = new char[interpreterPath.length() + 1];
	std::strcpy(argv[0], interpreterPath.c_str());
	argv[1] = new char[scriptPath.length() + 1];
	std::strcpy(argv[1], scriptPath.c_str());
	argv[2] = NULL;

	return argv;
}

Cgi::Cgi(Client *client) :
		_client(client),
		_currentBufferSize(0),
		_timeCreated(0),
		_isFinished(false),
		_errorCode(0),
		_bodyBuffer(_client->getBodyBuffer()),
		_header(_client->getHeaderObject()),
		_clientIp(_client->getIp()),
		_currentState(READING_BODY),
		_fd(_client->getFd()) {
	_sockets[0] = -1;
	_sockets[1] = -1;
	_contentLength = _bodyBuffer.size();
	executeCgi();
}

Cgi::~Cgi() {
	if (_sockets[0] != -1) {
		close(_sockets[0]);
	}
}

bool Cgi::isFinished() const {
	return _isFinished;
}

int Cgi::getErrorCode() const {
	return _errorCode;
}

void Cgi::executeCgi() {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _sockets) < 0) {
		LOG_ERROR_WITH_TAG("Failed to create socket pair", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	}

	LOG_DEBUG("CGI EXECUTING");
	// Add the socket to epoll event list and create an EventData object for it
	// try {
	// 	if (client->getEventHandler()->registerEvent(_sockets[0], CGI, client) < 0) {
	// 		LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
	// 		_isFinished = true;
	// 		_errorCode = 500;
	// 		return;
	// }
	// EventsData *eventData = client->getEventHandler()->createNewEvent(_sockets[0], CGI, client);
	// client->getEventHandler()->addEventToList(eventData);
	// epoll_event ev;
	// ev.events = EPOLLIN | EPOLLOUT;
	// ev.data.ptr = eventData;
	// if (epoll_ctl(client->getEventHandler()->getEpollFd(), EPOLL_CTL_ADD, _sockets[0], &ev) < 0) {
	// 	LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
	// 	_isFinished = true;
	// 	_errorCode = 500;
	// 	return;
	// }
	// } catch (std::bad_alloc &e) {
	// 	LOG_ERROR_WITH_TAG("Failed to allocate memory", "CGI");
	// 	_isFinished = true;
	// 	_errorCode = 500;
	// 	return;
	// }

	// Update the last modified time
	_timeCreated = std::time(0);

	// Fork the process
	pid_t pid = fork();
	if (pid < 0) {
		LOG_ERROR_WITH_TAG("Failed to fork", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	} else if (pid != 0) { // Parent process
		close(_sockets[1]); // Close child's end of the socket pair

	} else { // Child process
		executeChild();
	}
	// close(_sockets[0]);
}

int Cgi::executeChild() {
	close(_sockets[0]); // Close parent's end of the socket pair
	dup2(_sockets[1], STDIN_FILENO);
	dup2(_sockets[1], STDOUT_FILENO);
	close(_sockets[1]); // Close child's end of the socket pair

	std::string dir = Config::getInstance()->getCgiDir(_header.getPath(), _header.getHost());
	const char *cgiDir = dir.c_str();
	if (chdir(cgiDir) < 0) {
		LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
		perror("chdir failed:");
		std::exit(255);
	}

	// Set the enviroment variables
	char **envp = createEnviromentVariables();
	char **argv = createArguments();

	execve(argv[0], argv, envp);
	LOG_ERROR_WITH_TAG("Failed to execve CGI", "CGI");
	perror("execve failed:");
	std::exit(255);

	return 0;
}

void Cgi::readBody() {
	if (_header.getMethod() == "POST") {
		// Get unchunked bodydata
		if (_header.isTransferEncodingChunked()) {
			if (!decodeChunkedBody(_bodyBuffer, _sendToChildBuffer)) {
				_errorCode = 400;
				_currentState = SENDING_RESPONSE;
			}
		} else {
			// Get leftover bodydata from headerparsing
			if (_bodyBuffer.size()) {
				_sendToChildBuffer += _bodyBuffer;
				_bodyBuffer.clear();
				if (_contentLength == _sendToChildBuffer.size()) {
					_currentState = CREATE_CGI_PROCESS;
					return;
				}
			}
			// Read the rest of the body
			char buffer[BUFFER_SIZE + 1];
			ssize_t readSize = read(_fd, buffer, BUFFER_SIZE);
			if (readSize > 0) {
				buffer[readSize] = 0;
				_sendToChildBuffer += buffer;
				// Check if the body is bigger then the content-length
				if (_contentLength && _sendToChildBuffer.size() > _contentLength) {
					_errorCode = 400;
					_currentState = SENDING_RESPONSE;
				} else if (_contentLength == _sendToChildBuffer.size()) { // TODO: Could bug if we read the max buffersize and there is still stuff in the socket
					_currentState = CREATE_CGI_PROCESS;
				}
			} else if (readSize == -1 || readSize == 0) {
				_currentState = FINISHED;
			}
		}
	} else if (_header.getMethod() == "GET") {
		_currentState = CREATE_CGI_PROCESS;
		return;
	} else {
		_errorCode = 405;
		_currentState = SENDING_RESPONSE;
	}
}

void Cgi::process() {
	switch (_currentState) {
		case READING_BODY:
			LOG_DEBUG_WITH_TAG("Reading body", "CGI");
			readBody();
			break;
		case CREATE_CGI_PROCESS:
			LOG_DEBUG_WITH_TAG("Creating CGI process", "CGI");
			if (createCgiProcess() < 0) {
				_currentState = FINISHED;
			}
			_currentState = WAITING_FOR_CHILD;
			break;
		case WAITING_FOR_CHILD:
			LOG_DEBUG_WITH_TAG("Waiting for child", "CGI");
			break;
		case SENDING_RESPONSE:
			LOG_DEBUG_WITH_TAG("Sending response", "CGI");
			break;
		case FINISHED:
			LOG_DEBUG_WITH_TAG("Finished", "CGI");
			break;
	}
}

int Cgi::decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody) {
	if (bodyBuffer.empty()) {
		return 1;
	}
	std::stringstream bodyStream(bodyBuffer);
	for (std::string line; std::getline(bodyStream, line);) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1); // Remove the trailing \r
		size_t chunkSize;
		std::stringstream sizeStream(line);
		if (!(sizeStream >> std::hex >> chunkSize)) // Parse the chunk size
			return 1;
		if (chunkSize == 0) {
			// Check for final CRLF
			char crlf[2];
			if (!bodyStream.read(crlf, 2) || crlf[0] != '\r' || crlf[1] != '\n')
				return 1;
			break;
		}
		size_t oldSize = decodedBody.size();
		decodedBody.resize(oldSize + chunkSize); // Resize the decoded body buffer to fit the new chunk
		if (!bodyStream.read(&decodedBody[oldSize], chunkSize)) // Write the chunk data to the decoded body buffer
			return 1;
		// Check for CRLF after chunk
		char crlf[2];
		if (!bodyStream.read(crlf, 2) || crlf[0] != '\r' || crlf[1] != '\n')
			return 1;
	}
	return 0;
}

int Cgi::createCgiProcess() {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _sockets) < 0) {
		LOG_ERROR_WITH_TAG("Failed to create socket pair", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return -1;
	}
	if (EventHandler::getInstance().registerEvent(_sockets[0], CGI, _client) < 0) {
		LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
		_isFinished = true;
		_errorCode = 500;
		close(_sockets[0]);
		close(_sockets[1]);
		return -1;
	}
	_timeCreated = std::time(0);

	pid_t _childPid = fork();
	if (_childPid < 0) {
		LOG_ERROR_WITH_TAG("Failed to fork", "CGI");
		_isFinished = true;
		_errorCode = 500;
		close(_sockets[0]);
		close(_sockets[1]);
		return -1;
	} else if (_childPid != 0) { // Parent process
		close(_sockets[1]); // Close child's end of the socket pair
	} else { // Child process
		executeChild();
	}

	return 0;
}

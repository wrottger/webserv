#include "Cgi.hpp"
#include <cstdlib>
#include <cstring>

// TODO: Delete this function
std::string createCgiTestResponse() {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
							   "<body><h1>Kein pull request approve fuer Freddy!</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string _responseHttp = "HTTP/1.1 200 OK\r\n"
								"Content-Type: text/html; charset=UTF-8\r\n"
								"Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return _responseHttp;
}

// TODO: Delete this function
std::string Cgi::createErrorResponse(int errorCode) {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Error</title></head>"
							   "<body><h1>Error " +
			Utils::toString(errorCode) + "</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string responseHttp = "HTTP/1.1 " + Utils::toString(errorCode) + " Error\r\n"
																		  "Content-Type: text/html; charset=UTF-8\r\n"
																		  "Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return responseHttp;
}

// Creates the environment variables for the CGI script
char **Cgi::createEnviromentVariables() {
	std::vector<std::string> envp;

	envp.push_back("AUTH_TYPE=");
	envp.push_back("CONTENT_LENGTH=" + Utils::toString(_contentLength)); // FIXME: When it was unchunked it should be the size after decoding
	if (_header.isInHeader("content-type")) {
		std::string contentType = _header.getHeader("content-type");
		envp.push_back("CONTENT_TYPE=" + contentType);
	}
	envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envp.push_back("PATH_INFO=" + Config::getInstance().getFilePath(_header.getPath(), _header.getHost()));
	envp.push_back("PATH_TRANSLATED=");
	envp.push_back("QUERY_STRING=" + _header.getQuery());
	envp.push_back("REMOTE_ADDR=" + _clientIp);
	envp.push_back("REMOTE_HOST=" + _clientIp);
	// envp.push_back("REMOTE_IDENT=");
	envp.push_back("REQUEST_METHOD=" + _header.getMethod());
	envp.push_back("SCRIPT_NAME=" + Config::getInstance().getCgiScriptPath(_header));
	envp.push_back("SERVER_NAME=" + _header.getHost());
	envp.push_back("SERVER_PORT=" + Utils::toString(_header.getPort()));
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
		result[i] = new char[envp[i].length() + 1];
		std::strncpy(result[i], envp[i].c_str(), envp[i].length());
		result[i][envp[i].length()] = '\0';
	}
	result[envp.size()] = NULL;

	return result;
}

// Creates the arguments for the CGI script
char **Cgi::createArguments() {
	std::string interpreterPath = Config::getInstance().getCgiInterpreterPath(_header);
	if (interpreterPath.empty()) {
		perror("Failed to get interpreter path");
		exit(255);
	}
	std::string scriptPath = Config::getInstance().getCgiScriptPath(_header);
	if (scriptPath.empty()) {
		perror("Failed to get script path");
		exit(255);
	}
	LOG_TRACE_WITH_TAG(interpreterPath, "CGI");
	LOG_TRACE_WITH_TAG(scriptPath, "CGI");
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
		_header(_client->getHeaderObject()),
		_requestBody(_client->getBodyBuffer()),
		_clientIp(_client->getIp()),
		_fd(_client->getFd()),
		_currentCgiToServerBufferSize(0),
		_timeCreated(0),
		_errorCode(0),
		_state(CHECK_METHOD),
		_childPid(0),
		_eventData(NULL),
		_config(Config::getInstance()) {
	_sockets[0] = -1;
	_sockets[1] = -1;
	LOG_DEBUG_WITH_TAG("Cgi constructor called", "CGI");
	_contentLength = Utils::stringToNumber(_header.getContentLength());
}

Cgi::~Cgi() {
	LOG_DEBUG_WITH_TAG("Cgi destructor called", "CGI");
	// if (_sockets[0] != -1) {
	// 	close(_sockets[0]);
	// }
}

// Returns true if the CGI process is finished
bool Cgi::isFinished() const {
	return _state == FINISHED;
}

// Returns the error code of the CGI process
int Cgi::getErrorCode() const {
	return _errorCode;
}

// Executes the CGI script
int Cgi::executeChild() {
	close(_sockets[0]); // Close parent's end of the socket pair
	dup2(_sockets[1], STDIN_FILENO);
	dup2(_sockets[1], STDOUT_FILENO);
	close(_sockets[1]); // Close child's end of the socket pair

	std::string dir = Config::getInstance().getCgiDir(_header);
	const char *cgiDir = dir.c_str();
	if (chdir(cgiDir) < 0) {
		LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
		perror("chdir failed:");
		std::exit(255);
	}

	char **argv = createArguments();
	char **envp = createEnviromentVariables();

	if (execve(argv[0], argv, envp) == -1) {
		LOG_ERROR_WITH_TAG("Failed to execve CGI", "CGI");
		perror("execve failed:");
		std::exit(255);
	}

	return 0;
}

// Reads the body of the request
int Cgi::readBody(EventsData *eventData) {
	LOG_DEBUG_WITH_TAG("Reading body", "CGI");
	// Get unchunked bodydata
	if (_header.isTransferEncodingChunked()) {
		LOG_DEBUG_WITH_TAG("Reading chunked body", "CGI");
		if (!decodeChunkedBody(_requestBody, _serverToCgiBuffer)) {
			return -1;
		}
	} else {
		// Get leftover bodydata from headerparsing
		if (_requestBody.size()) {
			_serverToCgiBuffer += _requestBody;
			_requestBody.clear();
			if (_serverToCgiBuffer.size() == _contentLength) {
				LOG_DEBUG_WITH_TAG("Content-Length reached", "CGI");
				_state = CREATE_CGI_PROCESS;
				return 0;
				// Check if the body is bigger then the content-length and send error
			} else if (_serverToCgiBuffer.size() > _contentLength) {
				LOG_DEBUG_WITH_TAG("Content-Length exceeded", "CGI");
				return -1;
			}
		}
		// Read the rest of the body
		if (eventData->eventType == CLIENT) {
			if (eventData->eventMask & EPOLLIN) {
				// Read the body from the socket
				char buffer[BUFFER_SIZE + 1];
				ssize_t readSize = read(_fd, buffer, BUFFER_SIZE);
				std::cout << "Read size: " << readSize << std::endl;
				if (readSize > 0) {
					buffer[readSize] = 0;
					_serverToCgiBuffer += buffer;
					// Check if the body is bigger then the content-length
					if (_contentLength && _serverToCgiBuffer.size() > _contentLength) {
						return -1;
					} else if (_contentLength == _serverToCgiBuffer.size()) { // TODO: Could bug if we read the max buffersize and there is still stuff in the socket
						_state = CREATE_CGI_PROCESS;
					}
				} else if (readSize == -1 || readSize == 0) {
					_state = FINISHED;
				}
			} 
			// else {
			// 	// _state = CREATE_CGI_PROCESS;
			// 	LOG_DEBUG_WITH_TAG("Waiting for body", "CGI");
			// }
		}
	}
	LOG_DEBUG_WITH_TAG("Read body chunk done", "CGI");
	return 0;
}

// Sends the data to the child process
// return 1 if finished or no buffer return 0 if data was send, and -1 on error
int Cgi::sendToChild() {
	if (_serverToCgiBuffer.empty()) {
		return 1;
	}
	ssize_t sent = write(_sockets[0], _serverToCgiBuffer.c_str(), _serverToCgiBuffer.size());
	if (sent < 0) {
		LOG_ERROR_WITH_TAG("Failed to write to child", "CGI");
		kill(_childPid, SIGKILL);
		_state = SENDING_RESPONSE;
		_errorCode = 500;
		return -1;
	}
	_serverToCgiBuffer.erase(0, sent);
	return 0;
}

// Reads the data from the child process
// return bytes read
int Cgi::readFromChild() {
	char buffer[BUFFER_SIZE + 1];
	ssize_t readSize = recv(_sockets[0], buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (readSize > 0) {
		buffer[readSize] = 0;
		_cgiToServerBuffer += buffer;
		std::string debug("CGI TO SERVER BUFFER");
		debug += _cgiToServerBuffer;
		LOG_DEBUG_WITH_TAG( debug, "CGI");
	} else if (readSize == -1 || readSize == 0) {
		LOG_DEBUG_WITH_TAG( "READING DONE", "CGI");
		return 0;
	}
	return readSize;
}

// Checks if the method is allowed
// set right state for GET or POST/PUT Method return -1 if not
int Cgi::checkIfValidMethod() {
	// it is not only checking if method is valid, its also seeting state
	LOG_DEBUG_WITH_TAG("Checking method", "CGI");
	if (_header.getMethod() == "GET") {
		if (!Config::getInstance().isMethodAllowed(_header, "GET")) {
			LOG_DEBUG_WITH_TAG("GET Method not allowed", "CGI");
			return -1;
		}
		LOG_DEBUG_WITH_TAG("GET method called", "CGI");
		_state = CREATE_CGI_PROCESS;
	} else if (_header.getMethod() == "POST") {
		if (!Config::getInstance().isMethodAllowed(_header, "POST")) {
			LOG_DEBUG_WITH_TAG("POST Method not allowed", "CGI");
			return -1;
		}
		LOG_DEBUG_WITH_TAG("POST method called", "CGI");
		_state = READING_BODY;
	} else {
		LOG_DEBUG_WITH_TAG("Method not allowed", "CGI");
		return -1;
	}
	return 0;
}

// Processes the CGI request
void Cgi::process(EventsData *eventData) {
	int status = 0;
	int waitPidReturn = 0;
	switch (_state) {
		case CHECK_METHOD:
			if (checkIfValidMethod() < 0) {
				_errorCode = 405;
				_state = SENDING_RESPONSE;
				break;
			}
			if (checkIfValidFile() < 0) {
				_errorCode = 404;
				_state = SENDING_RESPONSE;
				break;
			}
			// Fallthrough!!!
		case READING_BODY:
			if (readBody(eventData) < 0) {
				_errorCode = 400;
				_state = SENDING_RESPONSE;
				break;
			}
			break;
		case CREATE_CGI_PROCESS:
			LOG_DEBUG_WITH_TAG("Creating CGI process", "CGI");
			if (createCgiProcess() < 0) {
				LOG_DEBUG_WITH_TAG("Failed to create CGI process", "CGI");
				_errorCode = 500;
				_state = SENDING_RESPONSE;
			}
			LOG_DEBUG_WITH_TAG("Created CGI process", "CGI");
			_state = SENDING_TO_CHILD;
			break;
		case SENDING_TO_CHILD:
			LOG_DEBUG_WITH_TAG("SENDING_TO_CHILD", "CGI");
			if (eventData->eventMask & EPOLLOUT && eventData->eventType == CGI) {
				LOG_DEBUG_WITH_TAG("Sending to child", "CGI");
				if (sendToChild() == 1) {
					_state = READING_FROM_CHILD;
				}
			}
			break;
		case READING_FROM_CHILD:
			// LOG_DEBUG_WITH_TAG("READING_FROM_CHILD", "CGI");
			if (eventData->eventMask & EPOLLIN && eventData->eventType == CGI) {
				LOG_DEBUG_WITH_TAG("Reading from child", "CGI");
				int readBytesFromChild = readFromChild();
				if (readBytesFromChild < 0) {
					LOG_DEBUG_WITH_TAG("Failed to read from child", "CGI");
					_errorCode = 500;
					_state = SENDING_RESPONSE;
				}
				else if (readBytesFromChild == 0) {
					LOG_DEBUG_WITH_TAG("Finished to read from child", "CGI");
					_state = WAITING_FOR_CHILD;
				}
			}
			// LOG_DEBUG_WITH_TAG("checking for timeout", "CGI");
			if (isTimedOut()) {
				LOG_DEBUG_WITH_TAG("Timeout reading from child", "CGI");
				kill(_childPid, SIGKILL); // Force quit the child process
				_state = WAITING_FOR_CHILD;
				// _errorCode = 500;
			}
			break;
		case WAITING_FOR_CHILD:
			LOG_DEBUG_WITH_TAG("WAITING_FOR_CHILD", "CGI");
			waitPidReturn = waitpid(_childPid, &status, WNOHANG);
			if (waitPidReturn == -1) {
				LOG_ERROR_WITH_TAG("Failed to wait for child", "CGI");
				LOG_ERROR_WITH_TAG(strerror(errno), "CGI");
				_state = SENDING_RESPONSE;
				_errorCode = 500;
			}
			else if (waitPidReturn > 0) {
				if (WIFEXITED(status)) {
					int exit_status = WEXITSTATUS(status);
					std::string exit_status_str = "Child exited with status " + Utils::toString(exit_status);
					LOG_DEBUG_WITH_TAG(exit_status_str, "CGI");
					if (exit_status == 0) {
						LOG_DEBUG_WITH_TAG("Child exited successfully", "CGI");
						_state = SENDING_RESPONSE;
					} else {
						LOG_DEBUG_WITH_TAG("Child exited with error", "CGI");
						_state = SENDING_RESPONSE;
						_errorCode = 500;
					}
				} 
				else if (WIFSIGNALED(status)) {
					LOG_DEBUG_WITH_TAG("Child signaled", "CGI");
					_state = SENDING_RESPONSE;
					_errorCode = 500;
				}
			}
			// _state = SENDING_RESPONSE;
			if (isTimedOut()) {
					LOG_DEBUG_WITH_TAG("Timeout waiting for child", "CGI");
					kill(_childPid, SIGKILL); // Force quit the child process
					// _state = SENDING_RESPONSE;
					// _errorCode = 500;
				}
			break;
		case SENDING_RESPONSE:
			LOG_DEBUG_WITH_TAG("SENDING_RESPONSE", "CGI");
			if (eventData->eventMask & EPOLLOUT && eventData->eventType == CLIENT) {
				LOG_DEBUG_WITH_TAG("Sending response", "CGI");
				if (_errorCode != 0) {
					LOG_DEBUG_WITH_TAG("Error response triggered", "CGI");
					_cgiToServerBuffer = createErrorResponse(_errorCode);
				}
				std::cout << "buffer to cgi: " << _cgiToServerBuffer << std::endl;
				if (send(_fd, _cgiToServerBuffer.c_str(), _cgiToServerBuffer.size(), 0) < 0) {
					LOG_ERROR_WITH_TAG("Failed to send response", "CGI");
				}
				_state = FINISHED;
			}
			break;
		case FINISHED:
			LOG_DEBUG_WITH_TAG("Finished", "CGI");
			break;
	}
}

// Returns the event data
EventsData *Cgi::getEventData() const {
	return _eventData;
}

// Creates the CGI process, socket connection and registers the event
int Cgi::createCgiProcess() {

	//test if folder then try to open

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _sockets) < 0) {
		LOG_ERROR_WITH_TAG("Failed to create socket pair", "CGI");
		return -1;
	}

	_eventData = EventHandler::getInstance().registerEvent(_sockets[0], CGI, _client);
	if (_eventData == NULL) {
		LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
		close(_sockets[0]);
		close(_sockets[1]);
		return -1;
	}

	LOG_DEBUG_WITH_TAG("TIME SET", "CGI");
	_timeCreated = std::time(0);

	_childPid = fork();
	if (_childPid < 0) {
		LOG_ERROR_WITH_TAG("Failed to fork", "CGI");
		close(_sockets[0]);
		close(_sockets[1]);
		return -1;
		// Parent process
	} else if (_childPid != 0) {
		close(_sockets[1]); // Close child's end of the socket pair
		// Child process
	} else {
		executeChild();
	}
	return 0;
}

// Checks if the file is accessable
int Cgi::checkIfValidFile() {
	LOG_DEBUG_WITH_TAG("Checking file", "CGI");
	std::ifstream getFile;
	std::string filePath = _config.getFilePath(_header.getPath(), _header.getHost()) + _config.getDirectiveValue(_header.getPath(), _header.getHost(), Config::Index);
	getFile.open(filePath.c_str());
	LOG_DEBUG_WITH_TAG(filePath.c_str(), "CGI");
	if (getFile.fail()) {
		LOG_ERROR_WITH_TAG("Couldnt open file", "CGI");
		return -1;
	}
	if (Utils::isFolder(filePath)) {
		LOG_ERROR_WITH_TAG("Is folder", "CGI");
		return -1;
	}
	getFile.close();
	return 0;
}

bool Cgi::isTimedOut() {
	if (std::time(0) - _timeCreated >= CGI_TIMEOUT) {
		return true;
	}
	return false;
}

// description: Decodes the chunked body and stores it in decodedBody until chunk size is 0;
// returns 0 if the last chunk is reached, 1 if more data is needed, -1 if an error occurs
int Cgi::decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody)
{
    static Cgi::decodeState state = READ_SIZE;
    static std::stringstream ss;
    static bool lastChunk = false;
    static unsigned int chunkSize = 0;

    if (lastChunk)
        return 0; // More data will not be processed
    for (size_t i = 0; i < bodyBuffer.size(); ++i)
    {
        switch (state)
        {
            case READ_SIZE:
            {
                if (bodyBuffer[i] == '\r')
                    state = READ_SIZE_END;
                else if (isxdigit(bodyBuffer[i]))
                    ss << bodyBuffer[i];
				else
					return -1;
                break;
            }
            case READ_SIZE_END:
            {
                if (bodyBuffer[i] == '\n')
                {
                    if (!(ss >> std::hex >> chunkSize) || !ss.eof())
                        return -1; // string to hex conversion failed
					ss.str("");
                    ss.clear();
                    if (chunkSize == 0)
                        lastChunk = true;
                    state = READ_CHUNK;
                }
                else
                    return -1;
                break;
            }
            case READ_CHUNK:
            {
                decodedBody.push_back(bodyBuffer[i]);
                chunkSize--;
                if (chunkSize == 0)
                    state = READ_TRAILER_CR;
                break;
            }
            case READ_TRAILER_CR:
            {
                if (bodyBuffer[i] == '\r')
                    state = READ_TRAILER_LF;
                else
                    return -1; // Invalid trailer
                break;
            }
            case READ_TRAILER_LF:
            {
                if (bodyBuffer[i] == '\n')
                {
                    if (lastChunk)
                        return 0; // Finished decoding
                    state = READ_SIZE;
                }
                else
                    return -1; // Invalid trailer
                break;
            }
        }
    }
    return 1; // More data needed
}
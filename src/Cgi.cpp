#include "Cgi.hpp"
#include <cstdlib>
#include <cstring>
#include "Utils.hpp"

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
	if (_header.isTransferEncodingChunked())
		envp.push_back("CONTENT_LENGTH=" + Utils::toString(_serverToCgiBuffer.size())); // FIXME: When it was unchunked it should be the size after decoding
	else
		envp.push_back("CONTENT_LENGTH=" + Utils::toString(_contentLength)); // FIXME: When it was unchunked it should be the size after decoding
	if (_header.isInHeader("content-type")) {
		std::string contentType = _header.getHeader("content-type");
		envp.push_back("CONTENT_TYPE=" + contentType);
	}
	envp.push_back("REDIRECT_STATUS=200");
	envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envp.push_back("PATH_INFO=" + Config::getInstance().getFilePath(_header));
	envp.push_back("SCRIPT_FILENAME=" + Config::getInstance().getFilePath(_header));
	envp.push_back("PATH_TRANSLATED=");
	envp.push_back("UPLOAD_PATH=" + Config::getInstance().getDir(_header) + Config::getInstance().getDirectiveValue(_header, Config::UploadDir));
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
		_bytesSendToCgi(0),
		_config(Config::getInstance()),
		_cgiResponse(_cgiToServerBuffer, _fd),
		_isInternalRedirect(false),
		_InternalRedirectLocation(""),
		_bodyBytesRead(_requestBody.size()),
		_maxBodySize(Config::getInstance().getMaxBodySize(_header)) {
			if (_requestBody.size() > _maxBodySize) {
				_errorCode = 413;
				LOG_DEBUG_WITH_TAG("Request body too large", "CGI");
				_state = SENDING_RESPONSE;
			}
	_sockets[0] = -1;
	_sockets[1] = -1;
	LOG_DEBUG_WITH_TAG("Cgi constructor called", "CGI");
	try {
		_contentLength = Utils::stringToNumber<size_t>(_header.getContentLength());
	} catch (std::invalid_argument &e) {
		_contentLength = 0;
	}
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
		int decoderStatus = _chunkedDecoder.decodeChunkedBody(_requestBody, _serverToCgiBuffer);
		if (decoderStatus == -1) {
			return -1;
		} else if (decoderStatus == 0) {
			_state = CREATE_CGI_PROCESS;
			return 0;
		} else if (decoderStatus == 1 && eventData->eventMask & EPOLLIN) {
			// Read the rest body from the socket
			_requestBody.clear();
			char buffer[BUFFER_SIZE + 1];
			ssize_t readSize = read(_fd, buffer, BUFFER_SIZE);
			if (readSize > 0) {
				buffer[readSize] = 0;
				for (size_t i = 0; i < static_cast<size_t>(readSize); i++) {
					_requestBody.push_back(buffer[i]);
				}
				if (_requestBody.size() > _maxBodySize) {
					_errorCode = 413;
					LOG_DEBUG_WITH_TAG("Request body too large", "CGI");
					_state = SENDING_RESPONSE;
					return 0;
				}
			} else if (readSize == -1 || readSize == 0) {
				LOG_DEBUG_WITH_TAG("Reading body read 0 or -1", "CGI");
				_state = FINISHED;
			}
		}
	} else {
		// Push leftover bodydata from header to serverToCgiBuffer
		if (_requestBody.size()) {
			for (size_t i = 0; i < _requestBody.size(); i++) {
				_serverToCgiBuffer.push_back(_requestBody[i]);
			}
			_requestBody.clear();
			if (_serverToCgiBuffer.size() == _contentLength) {
				LOG_DEBUG_WITH_TAG("Content-Length reached", "CGI");
				_state = CREATE_CGI_PROCESS;
				return 0;
				// Check if the body is bigger than the content-length and send error
			} else if (_serverToCgiBuffer.size() > _contentLength) {
				LOG_DEBUG_WITH_TAG("Content-Length exceeded", "CGI");
				return -1;
			}
		}
		// Read the rest of the body
		LOG_DEBUG_WITH_TAG("Reading rest of body", "CGI");
		if (eventData->eventType == CLIENT) {
			if (eventData->eventMask & EPOLLIN) {
				// Read the body from the socket
				char buffer[BUFFER_SIZE + 1];
				ssize_t readSize = read(_fd, buffer, BUFFER_SIZE);
				if (readSize > 0) {
					buffer[readSize] = 0;
					for (size_t i = 0; i < static_cast<size_t>(readSize); i++) {
						_serverToCgiBuffer.push_back(buffer[i]);
					}
					if (_serverToCgiBuffer.size() > _maxBodySize) {
						_errorCode = 413;
						LOG_DEBUG_WITH_TAG("Request body too large", "CGI");
						_state = SENDING_RESPONSE;
						return 0;
					}
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
	ssize_t sent;
	// if (_serverToCgiBuffer.empty()) {
	// 	return 1;
	if ((_bytesSendToCgi) == _serverToCgiBuffer.size()) {
		return 1;
	}
	if ((_bytesSendToCgi + BUFFER_SIZE) > _serverToCgiBuffer.size())
		sent = write(_sockets[0], &_serverToCgiBuffer[_bytesSendToCgi], _serverToCgiBuffer.size() - _bytesSendToCgi);
	else
		sent = write(_sockets[0], &_serverToCgiBuffer[_bytesSendToCgi], BUFFER_SIZE);
	// ssize_t sent = write(_sockets[0], _serverToCgiBuffer.data(), _serverToCgiBuffer.size());
	_bytesSendToCgi += sent;
	if (sent < 0) {
		LOG_ERROR_WITH_TAG("Failed to write to child", "CGI");
		kill(_childPid, SIGKILL);
		_state = SENDING_RESPONSE;
		_errorCode = 500;
		return -1;
	}
	// for (size_t i = 0; i < static_cast<size_t>(sent); i++) {
	// 	_serverToCgiBuffer.erase(_serverToCgiBuffer.begin());
	// }
	return 0;
}

// Reads the data from the child process
// return bytes read
int Cgi::readFromChild() {
	char buffer[BUFFER_SIZE + 1];
	ssize_t readSize = recv(_sockets[0], buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (readSize > 0) {
		buffer[readSize] = 0;
		_bodyBytesRead += readSize;
		if (_bodyBytesRead > MAX_CGI_BUFFER_SIZE) {
			LOG_ERROR_WITH_TAG("MAX_CGI_BUFFER_SIZE exceeded", "CGI");
			return -1;
		}
		_cgiToServerBuffer += buffer;
		// std::string debug("CGI TO SERVER BUFFER");
		// debug += _cgiToServerBuffer;
		// LOG_DEBUG_WITH_TAG(debug, "CGI");
	} else if (readSize == -1 || readSize == 0) {
		LOG_DEBUG_WITH_TAG("READING DONE", "CGI");
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
				LOG_DEBUG_WITH_TAG("Failed to read body", "CGI");
				_state = SENDING_RESPONSE;
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
			// LOG_DEBUG_WITH_TAG("SENDING_TO_CHILD", "CGI");
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
					LOG_DEBUG_WITH_TAG("Error reading from child", "CGI");
					_errorCode = 500;
					_state = SENDING_RESPONSE;
				} else if (readBytesFromChild == 0) {
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
			} else if (waitPidReturn > 0) {
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
				} else if (WIFSIGNALED(status)) {
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
				// std::cout << _cgiToServerBuffer.c_str() << std::endl;
				if (_errorCode != 0) {
					LOG_DEBUG_WITH_TAG("Error response triggered", "CGI");
					_cgiToServerBuffer = generateErrorResponse(_errorCode);
					if (send(_fd, _cgiToServerBuffer.c_str(), _cgiToServerBuffer.size(), 0) < 0) {
						LOG_ERROR_WITH_TAG("Failed to send response", "CGI");
					}
					_state = FINISHED;
					return;
				}
				if (_cgiResponse.sendResponse() == -1) {
					LOG_ERROR_WITH_TAG("Failed to send cgiobject response", "CGI");
					_errorCode = 500;
					break;
				}
				if (_cgiResponse.isFinished()) {
					if (_cgiResponse.isInternalRedirect()) {
						LOG_DEBUG_WITH_TAG("Internal redirect", "CGI");
						_isInternalRedirect = true;
						_InternalRedirectLocation = _cgiResponse.getInternalRedirectLocation();
					} else {
						LOG_DEBUG_WITH_TAG("Finished sending response", "CGI");
					}
					_state = FINISHED;
				}
				// _state = FINISHED;
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

bool Cgi::isInternalRedirect() const {
	return _isInternalRedirect;
}

std::string Cgi::getInternalRedirectLocation() const {
	return _InternalRedirectLocation;
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
	std::string filePath = _config.getFilePath(_header);
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

// Checks if the CGI process timed out
bool Cgi::isTimedOut() {
	if (std::time(0) - _timeCreated >= CGI_TIMEOUT) {
		return true;
	}
	return false;
}

std::string Cgi::generateErrorResponse(const int errorCode) {
	std::string error_path = _config.getErrorPage(errorCode, _header);
	std::string response;
	response = "HTTP/1.1 ";
	std::string message = getErrorMessage(errorCode);
	std::stringstream errCode;
	errCode << errorCode;
	if (error_path.empty())
	{
		std::string error_html = "<HTML><body><p><strong>";
		error_html += errCode.str();
		error_html += " </strong>";
		error_html += message;
		error_html += "</p></body>";

		response += errCode.str();
		response += " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: ";
		std::stringstream errSize;
		errSize << error_html.size();
		response += errSize.str();
		response += "\r\n\r\n";
		response += error_html;
	}
	else
	{
		std::ifstream file(error_path.c_str());
		if (!file.is_open())
		{
			std::string error_html = "<HTML><body><p><strong>";
			error_html += "500";
			error_html += " </strong>";
			error_html += "Couldn't open error file";
			error_html += "</p></body>";

			response += errCode.str();
			response += " ";
			response += "Couldn't open error file\r\n";
			response += "Connection: close\r\n";
			response += "Content-Type: text/html\r\n";
			response += "Content-Length: ";
			std::stringstream errSize;
			errSize << error_html.size();
			response += errSize.str();
			response += "\r\n\r\n";
			response += error_html;
			return response;
		}
		response += errCode.str() + " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n\r\n";
		response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	LOG_DEBUG_WITH_TAG("Generated error response", "CGI::generateErrorResponse");
	return response;
}

std::string Cgi::getErrorMessage(const int errorCode) {
	switch (errorCode) {
		case 500:
			return ("Internal Server Error");
		case 400:
			return ("Bad request");
		case 404:
			return ("Not found");
		case 405:
			return ("Method not allowed");
		default:
			return ("Error");
	}

}

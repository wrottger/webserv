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

std::vector<std::string> Cgi::createEnviromentVariables() {
	std::vector<std::string> result;
	std::string contentType = _headerObject->getHeader("content-type");

	result.push_back("AUTH_TYPE=");
	result.push_back("CONTENT_LENGTH=" + toString(_bodyLength));
	if (!contentType.empty()) {
		result.push_back("CONTENT_TYPE=" + contentType);
	}
	result.push_back("GATEWAY_INTERFACE=CGI/1.1");
	result.push_back("PATH_INFO=" + Config::getInstance()->getFilePath(_headerObject->getPath(), "localhost"));
	result.push_back("PATH_TRANSLATED=");
	result.push_back("QUERY_STRING=" + _headerObject->getQuery());
	result.push_back("REMOTE_ADDR=" + _clientIp);
	result.push_back("REMOTE_HOST=" + _clientIp);
	// result.push_back("REMOTE_IDENT=");
	result.push_back("REQUEST_METHOD=" + _headerObject->getMethod());
	result.push_back("SCRIPT_NAME=" + _headerObject->getPath()); // FIXME: Maybe wrong when: /path/to/script.py/param1/param2
	result.push_back("SERVER_NAME=" + _headerObject->getHeader("host"));
	// result.push_back("SERVER_PORT=" + _headerObject->getPort()); // TODO: Change to actual server port
	result.push_back("SERVER_PROTOCOL=HTTP/1.1");
	result.push_back("SERVER_SOFTWARE=WebServ/1.0");

	for (std::map<std::string, std::string>::const_iterator it = _headerObject->getHeaders().begin(); it != _headerObject->getHeaders().end(); ++it) {
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		std::replace(key.begin(), key.end(), '-', '_');
		result.push_back("HTTP_" + key + "=" + it->second);
	}

	return result;
}

char **Cgi::createArguments() {
	char **argv = new char *[3];
	argv[0] = strdup("/bin/python3");
	argv[1] = strdup("overflow.py");
	argv[2] = NULL; // The environment list must be NULL-terminated

	return argv;
}

Cgi::Cgi(const std::string &bodyBuffer, HttpHeader *headerObject, std::string clientIp) :
		_currentBufferSize(0),
		_timeCreated(0),
		_isFinished(false),
		_errorCode(0),
		_bodyBuffer(bodyBuffer),
		_headerObject(headerObject),
		_clientIp(clientIp){
	_sockets[0] = -1;
	_sockets[1] = -1;
	_bodyLength = bodyBuffer.size();
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
	// if (client->getEventHandler()->registerEvent(_sockets[0], CGI, client) < 0) {
	// 	LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
	// 	_isFinished = true;
	// 	_errorCode = 500;
	// 	return;
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
		executeChild(_headerObject);
	}
	// close(_sockets[0]);
}

int Cgi::executeChild(const HttpHeader *headerObject) {
	close(_sockets[0]); // Close parent's end of the socket pair
	dup2(_sockets[1], STDIN_FILENO);
	dup2(_sockets[1], STDOUT_FILENO);
	// close(_sockets[1]); // Close child's end of the socket pair

	// TODO: Implement change directory
	// if (chdir(Config::getInstance().getDirPath()) < 0) {
	// 	LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
	// 	std::exit(255);
	// }

	if (chdir("cgi/") < 0) {
		LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
		std::exit(255);
	}

	// Set the enviroment variables
	// char **envp = createEnvironment(headerObject);
	char **envp = NULL;
	(void)headerObject;
	char **argv = createArguments();

	execve(argv[0], argv, envp);
	LOG_ERROR_WITH_TAG("Failed to execve CGI", "CGI");
	perror("execve failed:"); // TODO: DELETE DEBUG

	std::exit(255);

	return 0;
}

int CGI::decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody)
{
    if (bodyBuffer.empty()) {
        return 1;
    }
    std::stringstream bodyStream(bodyBuffer);
    for (std::string line; std::getline(bodyStream, line);)
    {
		if (!line.empty() && line[line.size() - 1] == '\r')
		    line.erase(line.size() - 1); // Remove the trailing \r
		size_t chunkSize;
		std::stringstream sizeStream(line);
		if (!(sizeStream >> std::hex >> chunkSize)) // Parse the chunk size
		    return 1;
		if (chunkSize == 0)
		{
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

// int decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody)
// {
// 	if (bodyBuffer.empty()) {
// 		return 1;
// 	}
// 	std::stringstream bodyStream(bodyBuffer);
// 	for (std::string line; std::getline(bodyStream, line);)
// 	{
// 		size_t chunkSize;
// 		std::stringstream sizeStream(line);
// 		if (!(sizeStream >> std::hex >> chunkSize))
// 			return -1;
// 		if (chunkSize == 0)
// 			break;
// 		std::string chunk;
// 		chunk.resize(chunkSize);
// 		if (!bodyStream.read(&chunk[0], chunkSize))
// 			return -1;
// 		decodedBody += chunk;
// 	}
// 	return 0;
// }
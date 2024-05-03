#include "Cgi.hpp"
#include <cstdlib>
#include <cstring>

char *dupString(const std::string &str) {
	char *cstr = new char[str.length() + 1];
	std::strcpy(cstr, str.c_str());
	return cstr;
}

void createMetaEnviromentVariables(char **envp, size_t &pos) {
	envp[pos++] = dupString("AUTH_TYPE="); // TODO: Change to actual auth type
	envp[pos++] = dupString("CONTENT_LENGTH="); // TODO: Change to actual content length
	envp[pos++] = dupString("CONTENT_TYPE=application/x-www-form-urlencoded"); // TODO: Change to actual content type
	envp[pos++] = dupString("GATEWAY_INTERFACE=CGI/1.1");
	envp[pos++] = dupString("PATH_INFO=/path/to/script"); // TODO: Change to actual path info
	envp[pos++] = dupString("PATH_TRANSLATED=/path/to/script"); // TODO: Change to actual path translated
	envp[pos++] = dupString("QUERY_STRING=param1=value1&param2=value2"); // TODO: Change to actual query string
	envp[pos++] = dupString("REMOTE_ADDR="); // TODO: Change to actual remote address
	envp[pos++] = dupString("SERVER_SOFTWARE=WebServ/1.0");
	envp[pos++] = dupString("SERVER_NAME=localhost"); // TODO: Change to actual server name
	envp[pos++] = dupString("SERVER_PROTOCOL=HTTP/1.1");
	envp[pos++] = dupString("SERVER_PORT=8080"); // TODO: Change to actual server port
	envp[pos++] = dupString("REQUEST_METHOD=GET"); // TODO: Change to actual request method
	envp[pos++] = dupString("SCRIPT_NAME=/path/to/script"); // TODO: Change to actual script name


}

char **Cgi::createEnvironment(const HttpHeader *headerObject) {
	const std::map<std::string, std::string> headers = headerObject->getHeaders();
	char **env = new char *[headers.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
	std::string envVar = it->first + "=" + it->second;
			env[i] = strdup(envVar.c_str());
			++i;
	}
	// char **env = new char *[13];
	// env[0] = strdup("REQUEST_METHOD=GET");
	// env[1] = strdup("QUERY_STRING=param1=value1&param2=value2");
	// env[2] = strdup("CONTENT_TYPE=application/x-www-form-urlencoded");
	// env[3] = strdup("CONTENT_LENGTH=");
	// env[4] = strdup("SCRIPT_NAME=/path/to/script");
	// env[5] = strdup("REQUEST_URI=/path/to/script?param1=value1&param2=value2");
	// env[6] = strdup("DOCUMENT_URI=/path/to/script");
	// env[7] = strdup("DOCUMENT_ROOT=/path/to/webroot");
	// env[8] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	// env[9] = strdup("REMOTE_ADDR=127.0.0.1");
	// env[10] = strdup("SERVER_NAME=localhost");
	// env[11] = strdup("SERVER_PORT=80");
	// env[12] = NULL; // The environment list must be NULL-terminated

	return env;
}

char **Cgi::createArguments() {
	char **argv = new char *[3];
	argv[0] = strdup("/bin/python3");
	argv[1] = strdup("overflow.py");
	argv[2] = NULL; // The environment list must be NULL-terminated

	return argv;
}

Cgi::Cgi(const std::string &bodyBuffer, HttpHeader *headerObject) :
		_currentBufferSize(0),
		_timeCreated(0),
		_isFinished(false),
		_errorCode(0),
		_bodyBuffer(bodyBuffer),
		_headerObject(headerObject) {
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
	char **envp = createEnvironment(headerObject);
	char **argv = createArguments();

	execve(argv[0], argv, envp);
	LOG_ERROR_WITH_TAG("Failed to execve CGI", "CGI");
	perror("execve failed:"); // TODO: DELETE DEBUG

	std::exit(255);

	return 0;
}

#include "Cgi.hpp"

#include <cstdlib>
#include <cstring>

char** Cgi::createEnvironment(const HttpHeader *headerObject) {
    (void)headerObject;
	char** env = new char*[13];
    env[0] = strdup("REQUEST_METHOD=GET");
    env[1] = strdup("QUERY_STRING=param1=value1&param2=value2");
    env[2] = strdup("CONTENT_TYPE=application/x-www-form-urlencoded");
    env[3] = strdup("CONTENT_LENGTH=");
    env[4] = strdup("SCRIPT_NAME=/path/to/script");
    env[5] = strdup("REQUEST_URI=/path/to/script?param1=value1&param2=value2");
    env[6] = strdup("DOCUMENT_URI=/path/to/script");
    env[7] = strdup("DOCUMENT_ROOT=/path/to/webroot");
    env[8] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    env[9] = strdup("REMOTE_ADDR=127.0.0.1");
    env[10] = strdup("SERVER_NAME=localhost");
    env[11] = strdup("SERVER_PORT=80");
    env[12] = NULL; // The environment list must be NULL-terminated

    return env;
}

char **Cgi::createArguments() {
	    char** argv = new char*[3];
    argv[0] = strdup("/bin/python3");
    argv[1] = strdup("overflow.py");
    argv[2] = NULL; // The environment list must be NULL-terminated

	return argv;
}

Cgi::Cgi(const std::string &bodyBuffer, Client *client, EventsData * event) :
		_isFinished(false), _errorCode(0),
		_bodyBuffer(bodyBuffer),
		_client(client),
		_event(event) {
			_sockets[0] = -1;
			_sockets[1] = -1;
	executeCgi(client);
}

Cgi::~Cgi() {
	// TODO: fix epoll delete with epollfd
	if (_sockets[0] != -1) {
		// epoll_ctl(_client->getEventHandler()->getEpollFd(), EPOLL_CTL_DEL, _sockets[0], NULL);
		close(_sockets[0]);
	}
	// if (_sockets[1] != -1) {
	// 	epoll_ctl(_client->getEventHandler()->getEpollFd(), EPOLL_CTL_DEL, _sockets[1], NULL);
	// 	close(_sockets[1]);
	// }
	// _client->getEventHandler()->deleteFromList();
	// delete _event;
	// _client->getEventHandler()->unregisterEvent(_sockets[0]);
	_client->getEventHandler()->addToCleanUpList(_sockets[0]);
}

bool Cgi::isFinished() const {
	return _isFinished;
}

int Cgi::getErrorCode() const {
	return _errorCode;
}

void Cgi::executeCgi(Client *client) {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _sockets) < 0) {
		LOG_ERROR_WITH_TAG("Failed to create socket pair", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	}

	LOG_ALARM("ALARM CGI EXECUTING");
	// Add the socket to epoll event list and create an EventData object for it
	try {
		if (client->getEventHandler()->registerEvent(_sockets[0], CGI, client) < 0) {
			LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
			_isFinished = true;
			_errorCode = 500;
			return;
		}
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
	} catch (std::bad_alloc &e) {
		LOG_ERROR_WITH_TAG("Failed to allocate memory", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	}

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
		executeChild(client->getHeaderObject());
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

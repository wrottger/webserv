#include "Cgi.hpp"

const char *Cgi::createEnviroment() {
	return NULL;
}

Cgi::Cgi(const std::string &bodyBuffer, Client *client) :
		_isFinished(false), _errorCode(0), _sockets{ -1, -1 } {
	executeCgi(bodyBuffer, epollFd, requestObject);
}

Cgi::~Cgi() {
	if (_sockets[0] != -1) {
		epoll_ctl(_sockets[0], EPOLL_CTL_DEL, _sockets[0], NULL);
		close(_sockets[0]);
	}
	if (_sockets[1] != -1) {
		epoll_ctl(_sockets[1], EPOLL_CTL_DEL, _sockets[1], NULL);
		close(_sockets[1]);
	}
}

bool Cgi::isFinished() const {
	return _isFinished;
}

int Cgi::getErrorCode() const {
	return _errorCode;
}

void Cgi::executeCgi(const std::string &bodyBuffer, Client *client) {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _sockets) < 0) {
		LOG_ERROR_WITH_TAG("Failed to create socket pair", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	}

	// Add the socket to epoll event list and create an EventData object for it
	try {
		EventsData *eventData = client->getEventHandler()->createNewEvent(_sockets[0], CGI, client);
		client->getEventHandler()->addEventToList(eventData);
		epoll_event ev;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = eventData;
		if (epoll_ctl(client->getEventHandler()->getEpollFd(), EPOLL_CTL_ADD, _sockets[0], &ev) < 0) {
			LOG_ERROR_WITH_TAG("Failed to add socket to epoll", "CGI");
			_isFinished = true;
			_errorCode = 500;
			return;
		}
	} catch (std::bad_alloc &e) {
		LOG_ERROR_WITH_TAG("Failed to allocate memory", "CGI");
		_isFinished = true;
		_errorCode = 500;
		return;
	}

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
		executeChild(bodyBuffer, client->getHeaderObject());
	}
}

int Cgi::executeChild(const std::string &bodyBuffer, const HttpHeader *headerObject) {
	close(_sockets[0]); // Close parent's end of the socket pair
	dup2(_sockets[1], STDIN_FILENO);
	dup2(_sockets[1], STDOUT_FILENO);
	close(_sockets[1]); // Close child's end of the socket pair

	// TODO: Implement change directory
	// if (chdir(Config::getInstance().getDirPath()) < 0) {
	// 	LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
	// 	std::exit(255);
	// }

	if (chdir("cgi/") < 0) {
		LOG_ERROR_WITH_TAG("Failed to change directory", "CGI");
		std::exit(255);
	}

	// const char *envp[] = createEnviroment();

	return 0;
}

#include "Client.hpp"

Client::Client() {}

Client::Client(int fd, EventHandler *eventHandler) :
		_fd(fd),
		_eventHandler(eventHandler) {
	_requestObject = new HttpHeader;
	updateTime();
}

Client::~Client() {
	close(_fd);
	delete _requestObject;
}

int Client::getFd() {
	return _fd;
}

std::time_t Client::getLastModified() {
	return _lastModified;
}

void Client::updateTime() {
	_lastModified = std::time(0);
}

bool Client::isHeaderComplete() {
	return _requestObject->isComplete();
}

void Client::parseBuffer(const char *buffer) {
	_requestObject->parseBuffer(buffer);
}

Client &Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

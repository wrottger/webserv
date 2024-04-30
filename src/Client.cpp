#include "Client.hpp"

Client::Client() {}

Client::Client(int fd, EventHandler *eventHandler) :
		_fd(fd),
		_eventHandler(eventHandler) {
	_headerObject = new HttpHeader;
	updateTime();
}

Client::~Client() {
	close(_fd);
	delete _headerObject;
}

int Client::getFd() const {
	return _fd;
}

std::time_t Client::getLastModified() const {
	return _lastModified;
}

void Client::updateTime() {
	_lastModified = std::time(0);
}

bool Client::isHeaderComplete() const {
	return _headerObject->isComplete();
}

void Client::parseBuffer(const char *buffer) {
	_headerObject->parseBuffer(buffer);
}

EventHandler *Client::getEventHandler() const{
	return _eventHandler;
}

HttpHeader *Client::getHeaderObject() const {
	return _headerObject;
}

Client &Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

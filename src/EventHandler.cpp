#include "EventHandler.hpp"
#include <sstream>

EventHandler::EventHandler()
{
}

EventHandler::EventHandler(SocketHandling &sockets)
{
	_epollFd = sockets.getEpollFd();
	_listeningSockets = sockets.getOpenFds();
}

EventHandler::~EventHandler()
{
}



void EventHandler::start()
{
	struct epoll_event events[MAX_EVENTS];
	int	epollTriggerCount;
	char buffer[BUFFER_SIZE + 1] = {0};
	bool wasListenSocket;
	std::list<int> cleanUpList;
	

	while(true)
	{
		epollTriggerCount = epoll_wait(_epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (epollTriggerCount == -1) {
			throw std::runtime_error("EventHandler: epoll_wait failed.");
		}
		for (int n = 0; n < epollTriggerCount; ++n) {
			// checking if a listeningSocket was triggerd then accept new connection
			std::cout << "miau" << std::endl;
			wasListenSocket = isListeningSocketTriggered(events, n);
			// else read from the connection socket
			if (!wasListenSocket) {
				ssize_t bytes_received = read(events[n].data.fd, buffer, BUFFER_SIZE);
				std::cout << "bztes: " << bytes_received << std::endl;
				if (bytes_received == 0) {
					// The client has closed the connection
					// we need total destruction
					cleanUpList.push_back(events[n].data.fd);
					std::cout << "client connection closes" << std::endl;
					// epoll_ctl(_epollFd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
					// close(events[n].data.fd);
				}
				else if (bytes_received == -1) {
					cleanUpList.push_back(events[n].data.fd);
					std::cout << "client connection closes" << std::endl;
					// throw std::runtime_error("EventHandler: read failed.");
				}
				else {
					// find object with fd
					// parse
					buffer[bytes_received] = 0;
					for (std::list<EventHandler::ClientConnection *>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); it++) {
						if ((*it)->getFd() == events[n].data.fd) {
							std::cout << buffer << std::endl;
							(*it)->updateTime();
							std::string responseBody = 	"<!DOCTYPE html><html><head><title>Hello World</title></head>"
														"<body><h1>Hello, World!</h1></body></html>";

							std::ostringstream oss;
							oss << responseBody.size();

							std::string httpResponse = 	"HTTP/1.1 200 OK\r\n"
														"Content-Type: text/html; charset=UTF-8\r\n"
														"Content-Length: " + oss.str() + "\r\n\r\n"
														+ responseBody;
							if (send((*it)->getFd(), httpResponse.c_str(), httpResponse.size(), 0) == -1) {
								perror("Send");
							}
							if ((*it)->isHeaderComplete() && (*it)->isBodyComplete()) {
								// Reponse logic
								// Clientobjekt uebernimmt das eigene handling(Parsing check, response etc.)
								;
							}
						}
					}
				}
			}
		}
		handleToCloseConnections(cleanUpList);
		cleanUpList.clear();
		handleTimeouts();
	}
}

EventHandler::EventHandler(EventHandler const &other)
{
	(void) other;
}

EventHandler &EventHandler::operator=(EventHandler const &other)
{
	(void) other;
	return *this;
}

bool EventHandler::isListeningSocketTriggered(epoll_event events_arr[], int n)
{
	struct epoll_event ev;
	int newConnectionFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	size_t listenSocketSize = _listeningSockets.size();

	for (size_t i = 0; i < listenSocketSize; i++) {
		if (_listeningSockets[i] == events_arr[n].data.fd) {	
			newConnectionFd = accept(_listeningSockets[i], (struct sockaddr *) &addr, &addrlen);
			if (newConnectionFd == -1) {
				throw std::runtime_error("EventHandler: accept failed.");
			}	
			ev.events = EPOLLIN | EPOLLOUT;
			ev.data.fd = newConnectionFd;
			if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newConnectionFd, &ev) == -1) {
				throw std::runtime_error("EventHandler: epoll add failed.");
			}
			_clientConnections.push_back(new ClientConnection(newConnectionFd));
		return true;
		}
	}
	return false;
}

void EventHandler::handleTimeouts()
{
	time_t current_time = std::time(0);
	// std::cout << "Called " << std::endl;
	for (std::list<EventHandler::ClientConnection *>::iterator it = _clientConnections.begin(); it != _clientConnections.end();) {
		if (current_time - (*it)->getLastModified() > CLIENT_TIMEOUT) {
			std::cout << "Kicked client with FD: " << (*it)->getFd() << std::endl; 
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, (*it)->getFd(), NULL);
			delete *it;
			it = _clientConnections.erase(it);
        } else {
            it++;
        }
	}
}

void EventHandler::handleToCloseConnections(std::list<int> &cleanUpList)
{
	for (std::list<int>::iterator itCleanUp = cleanUpList.begin(); itCleanUp != cleanUpList.end(); itCleanUp++) {
		for (std::list<EventHandler::ClientConnection *>::iterator it = _clientConnections.begin(); it != _clientConnections.end();) {
			if (*itCleanUp == (*it)->getFd()) {
				std::cout << "Kicked client with FD: " << (*it)->getFd() << std::endl; 
				epoll_ctl(_epollFd, EPOLL_CTL_DEL, (*it)->getFd(), NULL);
				delete *it;
				it = _clientConnections.erase(it);
			} else {
				it++;
			}
		}
	}
}

// ClientConnection

EventHandler::ClientConnection::ClientConnection() {}

EventHandler::ClientConnection::ClientConnection(int fd) : _fd(fd)
{
	_requestObject = new HttpRequest;
	updateTime();
}

EventHandler::ClientConnection::~ClientConnection()
{
	close(_fd);
	delete _requestObject;
}

int EventHandler::ClientConnection::getFd()
{
	return _fd;
}

std::time_t EventHandler::ClientConnection::getLastModified()
{
	return _lastModified;
}

void EventHandler::ClientConnection::updateTime()
{
	_lastModified = std::time(0);
	std::cout << _lastModified << std::endl;
}

bool EventHandler::ClientConnection::isHeaderComplete()
{
	return _requestObject->isHeaderComplete();
}

bool EventHandler::ClientConnection::isBodyComplete()
{
	return _requestObject->isBodyComplete();
}

void EventHandler::ClientConnection::parseBuffer(const char *buffer)
{
	_requestObject->parseBuffer(buffer);
	// try {
	// } catch (... ) {
	// 	std::cerr << "bad alloc oder so" << std::endl;
	// }
}

EventHandler::ClientConnection &EventHandler::ClientConnection::operator=(ClientConnection const &other) {
	(void) other;
	return *this;
}

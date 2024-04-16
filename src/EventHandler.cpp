#include "EventHandler.hpp"

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
	struct epoll_event ev, events[MAX_EVENTS];
	int	epollTriggerCount, newConnectionFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	char buffer[BUFFER_SIZE + 1] = {0};
	bool flag = true;
	

	while(true)
	{
		epollTriggerCount = epoll_wait(_epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (epollTriggerCount == -1) {
			throw std::runtime_error("EventHandler: epoll_wait failed.");
		}
		for (int n = 0; n < epollTriggerCount; ++n) {
			// checking if a listeningSocket was triggerd then accept new connection
			flag = true;
			for (size_t i = 0; i < _listeningSockets.size(); i++) {
				if (_listeningSockets[i] == events[n].data.fd) {	
					newConnectionFd = accept(_listeningSockets[i], (struct sockaddr *) &addr, &addrlen);
					if (newConnectionFd == -1)
					{
						throw std::runtime_error("EventHandler: accept failed.");
					}
					ev.events = EPOLLIN;
					ev.data.fd = newConnectionFd;
					if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newConnectionFd, &ev) == -1)
					{
						throw std::runtime_error("EventHandler: epoll add failed.");
					}
					_clientConnections.push_back(new ClientConnection(newConnectionFd));
				flag = false;
				}
			}
				// else read from the connection socket
			if (flag) {
				ssize_t bytes_received = read(events[n].data.fd, buffer, BUFFER_SIZE);
				if (bytes_received == 0)
				{
					// The client has closed the connection
					std::cout << "client connection closes" << std::endl;
					close(events[n].data.fd);
				}
				else if (bytes_received == -1)
				{
					throw std::runtime_error("EventHandler: read failed.");
				}
				else {
					// find object with fd
					// parse
					for (std::list<EventHandler::ClientConnection *>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); it++) {
						if ((*it)->getFd() == events[n].data.fd) {
							std::cout << buffer << std::endl;
							// (*it)->parseBuffer(buffer); 
						}
					}
				}
			}
		}
	}
}

EventHandler::EventHandler(EventHandler const &other)
{
}

EventHandler &EventHandler::operator=(EventHandler const &other)
{
	return *this;
}


// ClientConnection

EventHandler::ClientConnection::ClientConnection() {}

EventHandler::ClientConnection::ClientConnection(int fd) : _fd(fd)
{
	updateTime();
}

EventHandler::ClientConnection::~ClientConnection()
{
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
}

bool EventHandler::ClientConnection::isHeaderComplete()
{
	return _requestObject.isHeaderComplete();
}

bool EventHandler::ClientConnection::isBodyComplete()
{
	return _requestObject.isBodyComplete();
}

void EventHandler::ClientConnection::parseBuffer(const char *buffer)
{
	_requestObject.parseBuffer(buffer);
	// try {
	// } catch (... ) {
	// 	std::cerr << "bad alloc oder so" << std::endl;
	// }
}

EventHandler::ClientConnection &EventHandler::ClientConnection::operator=(ClientConnection const &other) {
	return *this;
}

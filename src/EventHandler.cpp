#include "EventHandler.hpp"

EventHandler::EventHandler() {}

EventHandler::EventHandler(SocketHandling &sockets) {
	_epollFd = sockets.getEpollFd();
	_listeningSockets = sockets.getOpenFds();
}

EventHandler::~EventHandler() {}

void EventHandler::start() {
	struct epoll_event events[MAX_EVENTS];
	int epollTriggerCount;
	char buffer[BUFFER_SIZE + 1] = { 0 };
	bool wasListenSocket;
	std::list<int> cleanUpList;

	signal(SIGPIPE, SIG_IGN);
	while (true) {
		epollTriggerCount = epoll_wait(_epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (epollTriggerCount == -1) {
			LOG_ERROR("epoll_wait failed.");
			// std::cerr << "EventHandler: epoll_wait failed." << std::endl; // TODO: LOGGING
		}
		for (int n = 0; n < epollTriggerCount; ++n) {
			// Accept new client
			if ((wasListenSocket = isListeningSocketTriggered(events, n))) {
				acceptNewClient(events, n);
			}
			// Read from client
			if ((!wasListenSocket) && events[n].events & EPOLLIN) {
				ssize_t bytes_received = read(events[n].data.fd, buffer, BUFFER_SIZE);
				std::cout << "bytes: " << bytes_received << std::endl; // DELETE: DEBUG

				// The client has closed the connection
				if (bytes_received == 0) {
					cleanUpList.push_back(events[n].data.fd);
					LOG_INFO("Client connection closes 0");
					// std::cout << "client connection closes 0" << std::endl; // DELETE: DEBUG
				} else if (bytes_received == -1) {
					cleanUpList.push_back(events[n].data.fd);
					LOG_INFO("Client connection closes -1");
					// std::cout << "client connection closes -1" << std::endl; // DELETE: DEBUG
				} else {
					// find object with fd
					// parse
					buffer[bytes_received] = '\0';
					for (std::list<EventHandler::Client *>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); it++) {
						if ((*it)->getFd() == events[n].data.fd) {
							std::cout << buffer << std::endl; // DELETE: DEBUG
							// (*it)->parseBuffer(buffer);
							(*it)->updateTime();
						}
					}
				}
			}
			// Write to client
			if ((!wasListenSocket) && events[n].events & EPOLLOUT && !(events[n].events & EPOLLIN) ) {
				Client *client = findClient(events[n].data.fd);
				if (client == NULL) {
					LOG_ERROR("Client not found");
					continue;
				} else {
					client->updateTime();
					// Response // DELETE: DEBUG
					std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
												"<body><h1>Hello, World!</h1></body></html>";

					std::ostringstream oss;
					oss << responseBody.size();

					std::string httpResponse = "HTTP/1.1 200 OK\r\n"
												"Content-Type: text/html; charset=UTF-8\r\n"
												"Content-Length: " +
							oss.str() + "\r\n\r\n" + responseBody;
					// TODO: checken nach Header ob Methode ueberhaupt erlaubt
					if (client->isHeaderComplete() && client->isBodyComplete()) {
						// Reponse logic
						// Clientobjekt uebernimmt das eigene handling(Parsing check, response etc.)
						if (send((client)->getFd(), httpResponse.c_str(), httpResponse.size(), 0) == -1) {
							perror("Send");
							cleanUpList.push_back(events[n].data.fd);
						}
					}
					// cleanUpList.push_back(events[n].data.fd);
				}
			}
		}
		handleToCloseConnections(cleanUpList);
		cleanUpList.clear();
		handleTimeouts();
	}
}

EventHandler::EventHandler(EventHandler const &other) {
	(void)other;
}

EventHandler &EventHandler::operator=(EventHandler const &other) {
	(void)other;
	return *this;
}

bool EventHandler::isListeningSocketTriggered(epoll_event events_arr[], int n) const {
	size_t listenSocketSize = _listeningSockets.size();

	for (size_t i = 0; i < listenSocketSize; i++) {
		if (_listeningSockets[i] == events_arr[n].data.fd) {
			return true;
		}
	}
	return false;
}

void EventHandler::handleTimeouts() {
	time_t current_time = std::time(0);
	for (std::list<EventHandler::Client *>::iterator it = _clientConnections.begin(); it != _clientConnections.end();) {
		if (current_time - (*it)->getLastModified() > CLIENT_TIMEOUT) {
			LOG_INFO("Kicked timeout client");
			// std::cout << "Kicked timeout client with FD: " << (*it)->getFd() << std::endl;
			destroyClient(*it);
			it = _clientConnections.erase(it);
		} else {
			it++;
		}
	}
}

void EventHandler::handleToCloseConnections(std::list<int> &cleanUpList) {
	for (std::list<int>::iterator itCleanUp = cleanUpList.begin(); itCleanUp != cleanUpList.end(); itCleanUp++) {
		for (std::list<EventHandler::Client *>::iterator it = _clientConnections.begin(); it != _clientConnections.end();) {
			if (*itCleanUp == (*it)->getFd()) {
				LOG_ENABLE_FILE_LOGGING();
				LOG_INFO("Kicked client: close connection");
				LOG_DISABLE_FILE_LOGGING();
				// std::cout << "Kicked in CloseConnections client with FD: " << (*it)->getFd() << std::endl;
				destroyClient(*it);
				it = _clientConnections.erase(it);
			} else {
				it++;
			}
		}
	}
}

void EventHandler::destroyClient(EventHandler::Client *client) {
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, client->getFd(), NULL);
	delete client;
}

void EventHandler::acceptNewClient(epoll_event events_arr[], int n) {
	struct epoll_event ev;
	int newConnectionFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	size_t listenSocketSize = _listeningSockets.size();

	for (size_t i = 0; i < listenSocketSize; i++) {
		if (_listeningSockets[i] == events_arr[n].data.fd) {
			newConnectionFd = accept(_listeningSockets[i], (struct sockaddr *)&addr, &addrlen);
			if (newConnectionFd == -1) {
				LOG_ERROR("accept failed.");
				// std::cerr << "EventHandler: accept failed." << std::endl;
				return;
			}
			ev.events = EPOLLIN | EPOLLOUT;
			ev.data.fd = newConnectionFd;
			if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newConnectionFd, &ev) == -1) {
				close(newConnectionFd);
				LOG_ERROR("EventHandler: epoll ADD failed.");
				// std::cerr << "EventHandler: epoll ADD failed." << std::endl;
				return;
			}
			Client *newClient = NULL;
			try {
				newClient = new Client(newConnectionFd);
				_clientConnections.push_back(newClient);
			} catch (...) {
				delete newClient;
				if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, newConnectionFd, NULL) == -1) {
					LOG_ERROR("EventHandler: epoll DEL failed.");
					// std::cerr << "EventHandler: epoll DEL failed." << std::endl;
				}
				close(newConnectionFd);
			}
		}
	}
}

EventHandler::Client *EventHandler::findClient(int fd) {
	for (std::list<EventHandler::Client *>::iterator it = _clientConnections.begin(); it != _clientConnections.end(); it++) {
		if ((*it)->getFd() == fd) {
			return *it;
		}
	}
	return NULL;
}

/********************************************************************/
/*                          CLIENT                                  */
/********************************************************************/

EventHandler::Client::Client() {}

EventHandler::Client::Client(int fd) :
		_fd(fd) {
	_requestObject = new HttpRequest;
	updateTime();
}

EventHandler::Client::~Client() {
	close(_fd);
	delete _requestObject;
}

int EventHandler::Client::getFd() {
	return _fd;
}

std::time_t EventHandler::Client::getLastModified() {
	return _lastModified;
}

void EventHandler::Client::updateTime() {
	_lastModified = std::time(0);
}

bool EventHandler::Client::isHeaderComplete() {
	return _requestObject->isHeaderComplete();
}

bool EventHandler::Client::isBodyComplete() {
	return _requestObject->isBodyComplete();
}

void EventHandler::Client::parseBuffer(const char *buffer) {
	_requestObject->parseBuffer(buffer);
}

EventHandler::Client &EventHandler::Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

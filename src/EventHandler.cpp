#include "EventHandler.hpp"
#include "Client.hpp"

std::string createTestResponse() {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
							   "<body><h1>Hello, World!</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string httpResponse = "HTTP/1.1 200 OK\r\n"
							   "Content-Type: text/html; charset=UTF-8\r\n"
							   "Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return httpResponse;
}

EventHandler::EventHandler() {}

EventHandler::EventHandler(SocketHandling &sockets) {
	_epollFd = sockets.getEpollFd();
	_listeningSockets = sockets.getOpenFds();
}

EventHandler::~EventHandler() {
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		delete *it;
	}
	_eventDataList.clear();
}

void EventHandler::start() {
	struct epoll_event events[MAX_EVENTS];
	int epollTriggerCount;
	bool testCgiOnce = true;

	Cgi *testCgi = NULL;

	signal(SIGPIPE, SIG_IGN);
	while (true) {
		epollTriggerCount = epoll_wait(_epollFd, events, MAX_EVENTS, -1);
		if (epollTriggerCount == -1) {
			LOG_ERROR("epoll_wait failed.");
			continue;
		}
		for (int n = 0; n < epollTriggerCount; ++n) {
			EventsData *eventData = static_cast<EventsData *>(events[n].data.ptr);
			switch (eventData->eventType) {
				case LISTENING:
					acceptNewClient(eventData);
					continue;
				case CLIENT:
					if (events[n].events & EPOLLIN) {
						Client *client = static_cast<Client *>((*eventData).objectPointer);
						readFromClient(*eventData);
						// CGI Test
						std::string testbody = "miau kakao body";
						if (testCgiOnce) {
							testCgiOnce = false;
							testCgi = new Cgi(testbody, client, eventData);
							(void)testCgi;
						}
						continue;
					}
					if (events[n].events & EPOLLOUT) {
						Client *client = static_cast<Client *>((*eventData).objectPointer);
						if (client->isHeaderComplete()) {
							std::string httpResponse = createTestResponse();
							if (client->getHeaderObject()->getMethod() == "GET") {
								if (send((client)->getFd(), httpResponse.c_str(), httpResponse.size(), 0) == -1) {
									LOG_ERROR_WITH_TAG("send failed", "EventHandler");
								}
							}
							if (client->getHeaderObject()->getMethod() == "POST") {
								// std::string testbody = "miau kakao body";
								// Cgi test(testbody, client);
								// Clientobjekt uebernimmt das eigene handling(Parsing check, response etc.)
								// client->updateTime();
								// _cleanUpList.push_back(events[n].data.fd);
							}
							// _cleanUpList.push_back(events[n].data.fd);
						}
					}
					break;
				case CGI:
					if (events[n].events & EPOLLIN) {
						LOG_DEBUG("CGI triggered");
						char buffer[BUFFER_SIZE + 1] = { 0 };
						Client *client = static_cast<Client *>((*eventData).objectPointer);
						ssize_t bytes_received = read(client->getFd(), buffer, BUFFER_SIZE);
						// The client has closed the connection
						if (bytes_received == 0) {
							delete testCgi;
							LOG_DEBUG("CGI connection closes 0");
						} else if (bytes_received == -1) {
							LOG_DEBUG("CGI connection closes -1");
							delete testCgi;
						} else {
							buffer[bytes_received] = '\0';
							LOG_DEBUG_WITH_TAG("CGI buffer", buffer);
						}
					}
					break;
			}
		}
		processCleanUpList();
		_cleanUpList.clear();
		// handleTimeouts();
	}
}

EventHandler::EventHandler(EventHandler const &other) {
	(void)other;
}

EventHandler &EventHandler::operator=(EventHandler const &other) {
	(void)other;
	return *this;
}

// void EventHandler::handleClientTimeouts() { // TODO: rethink Timeout logic
// 	time_t current_time = std::time(0);
// 	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end();) {
// 		if (current_time - (*it)->getLastModified() > CLIENT_TIMEOUT) {
// 			LOG_DEBUG("Kicked timeout client");
// 			destroyClient(*it);
// 			it = _eventDataList.erase(it);
// 		} else {
// 			it++;
// 		}
// 	}
// }

void EventHandler::processCleanUpList() {
	for (std::list<EventsData *>::iterator itCleanUp = _cleanUpList.begin(); itCleanUp != _cleanUpList.end(); itCleanUp++) {
		if ((*itCleanUp)->eventType == CLIENT) {
			delete static_cast<Client *>((*itCleanUp)->objectPointer);
		}
	}
	for (std::list<EventsData *>::iterator itCleanUp = _cleanUpList.begin(); itCleanUp != _cleanUpList.end(); itCleanUp++) {
		unregisterEvent(*itCleanUp);
	}
}

// Accept and add new client to epoll and client list
void EventHandler::acceptNewClient(EventsData *eventData) {
	int newConnectionFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	newConnectionFd = accept(eventData->fd, (struct sockaddr *)&addr, &addrlen);
	if (newConnectionFd == -1) {
		LOG_ERROR("EventHandler: accept failed.");
		return;
	}
	Client *newClient = NULL;
	try {
		newClient = new Client(newConnectionFd, this);
		registerEvent(newConnectionFd, CLIENT, newClient);
		LOG_DEBUG("New client connection");
	} catch (...) {
		delete newClient;
		close(newConnectionFd);
	}
}

void EventHandler::readFromClient(EventsData &eventData) {
	char buffer[BUFFER_SIZE + 1] = { 0 };
	ssize_t bytes_received = read(eventData.fd, buffer, BUFFER_SIZE);
	// The client has closed the connection
	if (bytes_received == 0) {
		_cleanUpList.push_back(&eventData);
		(void)_cleanUpList;
		LOG_DEBUG("Client connection closes 0");
	} else if (bytes_received == -1) {
		_cleanUpList.push_back(&eventData);
		LOG_DEBUG("Client connection closes -1");
	} else {
		buffer[bytes_received] = '\0';
		Client *client = static_cast<Client *>(eventData.objectPointer);
		client->updateTime();
		client->parseBuffer(buffer);
		std::string bufferDebug(buffer); //TODO: DELETE DEBUG
		LOG_BUFFER(bufferDebug);
	}
}

EventsData *EventHandler::createNewEvent(int fd, EventType type, Client *client) {
	EventsData *eventData = new EventsData;
	eventData->fd = fd;
	eventData->eventType = type;
	if (type == CLIENT || type == CGI) {
		eventData->objectPointer = client;
	} else {
		eventData->objectPointer = NULL;
	}
	return eventData;
}

void EventHandler::addEventToList(EventsData *eventData) {
	_eventDataList.push_back(eventData);
}

int EventHandler::getEpollFd() const {
	return _epollFd;
}

int EventHandler::registerEvent(int fd, EventType type, Client *client) {
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.ptr = createNewEvent(fd, type, client);
	_eventDataList.push_back(static_cast<EventsData *>(ev.data.ptr));
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		LOG_ERROR("RegisterEvent: epoll ADD failed.");
		return -1;
	}
	return 0;
}

void EventHandler::unregisterEvent(int fd) {
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		std::cerr << "Fd: " << fd << std::endl; // TODO: DELETE
		LOG_ERROR("UnregisterEvent: epoll DEL failed.");
	}
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		if ((*it)->fd == fd) {
			LOG_ALARM_WITH_TAG("Unregistered something with fd", "EventHandler");
			delete *it;
			_eventDataList.erase(it);
			break;
		}
	}
}

void EventHandler::unregisterEvent(EventsData *eventData) {
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, eventData->fd, NULL) == -1) {
		perror("epoll_ctl");
		std::cerr << "Fd: " << eventData->fd << std::endl; // TODO: DELETE
		LOG_ERROR("UnregisterEvent: epoll DEL failed.");
		perror("epoll_ctl");
	}
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		if (*it == eventData) {
			LOG_ALARM_WITH_TAG("Unregistered something with pointer", "EventHandler");
			delete *it;
			_eventDataList.erase(it);
			break;
		}
	}
}

void EventHandler::addToCleanUpList(int fd) {
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		if ((*it)->fd == fd) {
			_cleanUpList.push_back(*it);
			break;
		}
	}
}

void EventHandler::addToCleanUpList(EventsData *eventData) {
	_cleanUpList.push_back(eventData);
}

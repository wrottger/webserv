#include "EventHandler.hpp"
#include "Client.hpp"

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
	std::list<EventsData *> cleanUpList;
	bool testCgiOnce = true;

	Cgi *testCgi = NULL;

	signal(SIGPIPE, SIG_IGN);
	while (true) {
		epollTriggerCount = epoll_wait(_epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (epollTriggerCount == -1) {
			LOG_ERROR("epoll_wait failed.");
			continue;
		}
		for (int n = 0; n < epollTriggerCount; ++n) {
			EventsData *eventData = static_cast<EventsData *>(events[n].data.ptr);
			// Accept new client
			switch (eventData->eventType) {
				case LISTENING:
					acceptNewClient(eventData);
					continue;
				case CLIENT:
					if (events[n].events & EPOLLIN) {
						Client *client = static_cast<Client *>((*eventData).objectPointer);
						readFromClient(*eventData, cleanUpList);
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
						if (client->isHeaderComplete()) {
							// Reponse logic
							if (client->getHeaderObject()->getMethod() == "GET") {
								if (send((client)->getFd(), httpResponse.c_str(), httpResponse.size(), 0) == -1) {
									perror("Send");
								}
							}
							if (client->getHeaderObject()->getMethod() == "POST") {
								// std::string testbody = "miau kakao body";
								// Cgi test(testbody, client);
							// Clientobjekt uebernimmt das eigene handling(Parsing check, response etc.)
							// client->updateTime();
								// cleanUpList.push_back(events[n].data.fd);
							}
							// cleanUpList.push_back(events[n].data.fd);
						}
					}
					break;
				case CGI:
					if (events[n].events & EPOLLIN) {
						LOG_DEBUG("CGI triggered");
						char buffer[BUFFER_SIZE + 1] = { 0 };
						Client *client = static_cast<Client *>((*eventData).objectPointer);
						// std::cout << "EventType: " << eventData->eventType << std::endl;
						ssize_t bytes_received = read(client->getFd(), buffer, BUFFER_SIZE);
						// The client has closed the connection
						if (bytes_received == 0) {
							cleanUpList.push_back(eventData);
							std::cout << "adding to cleanuplist: " << eventData->eventType << std::endl;
							delete testCgi;
							LOG_DEBUG("CGI connection closes 0");
						} else if (bytes_received == -1) {
							cleanUpList.push_back(eventData);
							std::cout << "adding to cleanuplist: " << eventData->eventType << std::endl;
							LOG_DEBUG("CGI connection closes -1");
							delete testCgi;
						} else {
							buffer[bytes_received] = '\0';
							std::string bufferDebug(buffer); //TODO: DELETE DEBUG
							LOG_BUFFER(bufferDebug);
						}
					}
					break;
			}
		}
		processCleanUpList(cleanUpList);
		cleanUpList.clear();
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

void EventHandler::processCleanUpList(std::list<EventsData *> &cleanUpList) {
	for (std::list<EventsData *>::iterator itCleanUp = cleanUpList.begin(); itCleanUp != cleanUpList.end(); itCleanUp++) {
		for (std::list<EventsData *>::iterator itAllEvents = _eventDataList.begin(); itAllEvents != _eventDataList.end(); itAllEvents++) {
			std::cout << "inside is: " << (*itCleanUp)->eventType << std::endl;
			if (*itCleanUp == *itAllEvents) {
				std::cout << "cleaning: " << (*itCleanUp)->eventType << std::endl;
				if ((*itCleanUp)->eventType == CLIENT) {
					destroyClient(static_cast<Client *>((*itCleanUp)->objectPointer));
				}
				delete *itCleanUp;
				itAllEvents = _eventDataList.erase(itAllEvents);
			}
		}
	}
}

void EventHandler::destroyClient(Client *client) {
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, client->getFd(), NULL);
	delete client;
}

// Accept and add new client to epoll and client list
void EventHandler::acceptNewClient(EventsData *eventData) {
	struct epoll_event ev;
	int newConnectionFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	EventsData *newData;
	newConnectionFd = accept(eventData->fd, (struct sockaddr *)&addr, &addrlen);
	if (newConnectionFd == -1) {
		LOG_ERROR("EventHandler: accept failed.");
		return;
	}
	ev.events = EPOLLIN | EPOLLOUT;
	Client *newClient = NULL;
	try {
		newData = new EventsData;
		newData->fd = newConnectionFd;
		newData->eventType = CLIENT;
		newData->objectPointer = NULL;
				ev.data.ptr = newData;
		_eventDataList.push_back(newData);
		newClient = new Client(newConnectionFd, this);
		newData->objectPointer = newClient;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newConnectionFd, &ev) == -1) {
			LOG_ERROR("EventHandler: epoll ADD failed.");
			throw std::runtime_error("EventHandler: epoll ADD failed.");
			return;
		}
		LOG_DEBUG("New client connection");
	} catch (...) {
		delete newData;
		delete newClient;
		close(newConnectionFd);
	}
}

void EventHandler::readFromClient(EventsData &eventData, std::list<EventsData *> &cleanUpList) {
	char buffer[BUFFER_SIZE + 1] = { 0 };
	ssize_t bytes_received = read(eventData.fd, buffer, BUFFER_SIZE);
	// The client has closed the connection
	if (bytes_received == 0) {
		cleanUpList.push_back(&eventData);
		(void)cleanUpList;
		LOG_DEBUG("Client connection closes 0");
	} else if (bytes_received == -1) {
		cleanUpList.push_back(&eventData);
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
	eventData->objectPointer = client;
		return eventData;
}

void EventHandler::addEventToList(EventsData *eventData) {
	_eventDataList.push_back(eventData);
	std::cout << "Added: " << eventData->eventType << std::endl; // TODO: Remove
}

int EventHandler::getEpollFd() const {
	return _epollFd;
}

void EventHandler::registerEvent(int fd, EventType type, Client *client) {
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.ptr = createNewEvent(fd, type, client);
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		LOG_ERROR("EventHandler: epoll ADD failed.");
	}
	_eventDataList.push_back(static_cast<EventsData *>(ev.data.ptr));
}

void EventHandler::unregisterEvent(int fd) {
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		if ((*it)->fd == fd) {
			delete *it;
			_eventDataList.erase(it);
			break;
		}
	}
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
}

void EventHandler::unregisterEvent(EventsData *eventData) {
	for (std::list<EventsData *>::iterator it = _eventDataList.begin(); it != _eventDataList.end(); it++) {
		if (*it == eventData) {
			delete *it;
			_eventDataList.erase(it);
			break;
		}
	}
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, eventData->fd, NULL);
}

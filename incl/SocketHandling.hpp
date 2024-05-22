#ifndef SOCKETHANDLING_HPP
#define SOCKETHANDLING_HPP

#include "Config.hpp"
#include "EventsData.hpp"
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h> // für epoll_create1()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // For close()
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class SocketHandling {
private:
	std::vector<Config::ServerBlock> &_config;
	std::vector<int> _openFds;
	int _epollFd;
	std::list<EventsData *> eventDataList;

	SocketHandling(SocketHandling const &other);
	SocketHandling operator=(SocketHandling const &other);

	void setUpSocket(int port);
	void setUpEpoll();

public:
	SocketHandling(std::vector<Config::ServerBlock> &config);
	~SocketHandling();
	int getEpollFd();
	std::vector<int> getOpenFds();
};

#endif
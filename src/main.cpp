#include <iostream>
#include <string>
#include <cstring> // For memset
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // For close()
#include <sstream>
#include <fstream>
#include <sys/epoll.h>  // für epoll_create1()
#include <vector>
#include "SocketHandling.hpp"
#include "EventHandler.hpp"
#include "Config.hpp"

int main()
{
	// loadConfig
	configObject server1;
	// size_t serverCount = 3;
	std::vector<configObject> config;

	config.push_back(server1);
	SocketHandling sockets(config);
	EventHandler event(sockets);
	event.start();
}
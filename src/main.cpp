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
#include "Config.hpp"

#define MAX_EVENTS 10

int main()
{
	// loadConfig
	configObject server1(8080, 2342);
	// configObject server2(8081, 2342);
	// configObject server3(8082, 2342);
	size_t serverCount = 3;
	std::vector<configObject> pommes;

	pommes.push_back(server1);
	SocketHandling a(pommes);
	for (size_t i = 0; i < serverCount; i++)
	{
		std::cout << a.openFds[i] << std::endl;
	}

	// init Server
	// config Server port etc
	// listen

	// add epoll

	// epoll loop
		// exit handling
	

}
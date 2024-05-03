#include "Config.hpp"
#include "EventHandler.hpp"
#include "Logger.hpp"
#include "SocketHandling.hpp"
#include "colors.hpp"
#include "utest.h"
#include <netinet/in.h>
#include <sys/epoll.h> // für epoll_create1()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // For close()
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

UTEST_STATE();

int main(int argc, char *argv[], char *envp[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	Config *config = Config::getInstance();
	try {
		config->parseConfigFile(argv[1]);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if (!config->isLoaded())
		return 1;
	config->printProgressBar(1, 1);
	std::cout << GBOLD("\nConfig file loaded successfully") << std::endl;

	LOG_INFO("Server started");
	// LOG_SET_LOG_LEVEL(Logging::DISABLE_LOG);
	// LOG_DISABLE_CONSOLE_LOGGING();
	// LOG_SET_LOG_TARGET(Logging::LOG_TO_FILE);
	SocketHandling sockets(config->getServerBlocks());
	EventHandler event(sockets);
	event.start();
}
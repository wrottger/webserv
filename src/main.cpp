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
#include "colors.hpp"
#include "utest.h"
#include "Logger.hpp"

UTEST_STATE();

int main(int argc, char *argv[], char *envp[])
{
  if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	Config* config = Config::getInstance();
	try {
		config->parseConfigFile(argv[1]);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if (!config->isLoaded())
	  return 1;
  config->printProgressBar(1, 1);
  std::cout << GBOLD("\nConfig file loaded successfully") << std::endl;
  LOG_INFO("Server started");
  SocketHandling sockets(config->getServerBlocks());
  EventHandler event(sockets);
  event.start();
}
#ifndef SOCKETHANDLING_HPP
# define SOCKETHANDLING_HPP

# include <iostream>
# include <string>
# include <cstring> // For memset
# include <cstdlib> // For exit() and EXIT_FAILURE
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h> // For close()
# include <sstream>
# include <fstream>
# include <sys/epoll.h>  // für epoll_create1()
# include <vector>
# include <stdexcept>
# include "Config.hpp"

class SocketHandling
{
	private:

		std::vector<Config::ServerBlock> &_config;
		std::vector<int> _openFds;
		int _epollFd;

		SocketHandling(SocketHandling const &other);
		SocketHandling operator =(SocketHandling const &other);

		void setUpSocket(int port);
		void setUpEpoll();

		public:
		SocketHandling(std::vector<Config::ServerBlock> &config);
		~SocketHandling();
		int getEpollFd();
		std::vector<int> getOpenFds();
};

#endif
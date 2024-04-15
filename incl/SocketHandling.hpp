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
		SocketHandling(SocketHandling const &source);
		SocketHandling operator =(SocketHandling const &source);
		std::vector<configObject> &config;
		int	epollFd;
		
	public:
		std::vector<int> openFds;
		SocketHandling(std::vector<configObject> &config);
		~SocketHandling();
		void setUpSocket(int port);
};

#endif
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>

class configObject
{
	public:	
		int port;
		int serverFd;
		static const int serverCount = 3;

		configObject(int port, int serverFd): port(port), serverFd(serverFd){};
		int getServerCount() {return serverCount;};
		std::vector<int> getPorts() {return std::vector<int>{8080, 8081, 8082};};
};

#endif //CONFIG_HPP
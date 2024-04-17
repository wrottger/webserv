#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>

class configObject
{
	public:	
		int port;
		int serverFd;
		static const int serverCount = 3;
		std::vector<int> ports;

		configObject() {ports.push_back(8080);ports.push_back(8081);ports.push_back(8082);};
		int getServerCount() {
			return serverCount;
			};
		std::vector<int> getPorts() {
			return ports;
			};
};

#endif //CONFIG_HPP
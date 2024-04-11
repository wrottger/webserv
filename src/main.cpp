#include "Config.hpp"
#include "error.hpp"

int main (int argc, char *argv[], char *envp[])
{
    if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	Config config(argv[1]);
	try {
		config.openConfigFile(config.getFileName());
		config.scanTokens(config.getFileContent());
		std::vector<Node> nodes = config.getNodes();
		for (std::vector<Node>::iterator it = nodes.begin(); it != nodes.end(); it++)
		{
			std::cout << "Token: " << it->_token << " Value: " << it->_value << " Offset: " << it->_offset << std::endl;
		}
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
    return 0;
}
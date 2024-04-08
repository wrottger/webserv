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
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << config;
	try {
		config.parseConfigFile();
	}
	catch (Config::ConfigException &e) {
		std::cerr << "Error: " << e.what() << " at row " << e.getRow() << ", column " << e.getColumn() << std::endl;
		return 1;
	}
    return 0;
}
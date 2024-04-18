#include "Config.hpp"
#include "error.hpp"
#include "colors.hpp"

int main (int argc, char *argv[], char *envp[])
{
    if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	Config config;
	try {
		config.parseConfigFile(argv[1]);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if (config.isLoaded())
	{
		std::cout << GBOLD("Config file loaded successfully") << std::endl;
	}
    return 0;
}
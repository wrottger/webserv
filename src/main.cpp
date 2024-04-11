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
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
    return 0;
}
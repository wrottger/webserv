#include "ConfigParser.hpp"
#include "Error.hpp"

int main (int argc, char *argv[], char *envp[])
{
    if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	std::fstream file;
	file.open(argv[1]);
	if (!file.is_open())
	{
		std::cerr << "Error: " << streamState(file) << std::endl;
		return 1;
	}
	file.close();
	// ConfigParser configParser(argv[1]);
    return 0;
}
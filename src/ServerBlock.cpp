#include "ServerBlock.hpp"
#include <sstream> // Include the necessary header file

std::vector<int> ServerBlock::getPort(void)
{
    std::vector<int> ports;

    for (std::vector<std::pair<TokenType, std::string> >::iterator it = _directives.begin(); it != _directives.end(); it++)
    {
        if (it->first == Port)
        {
            std::stringstream ss(it->second);
            int temp;
            ss >> temp;
            ports.push_back(temp);
        }
    }
    return ports;
}
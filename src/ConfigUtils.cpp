#include "Config.hpp"
#include "colors.hpp"

Config::Config(void) : _isLoaded(false)
{

}

bool Config::isLoaded(void) const
{
    return _isLoaded;
}

const std::vector<Node>& Config::getNodes(void) const
{
    return _nodes;
}

Config* Config::getInstance()
{
        if (_instance == NULL) {
            _instance = new Config();
        }
        return _instance;
}

void Config::error(const std::string &msg, const std::vector<Node>::iterator& it)
{
	std::stringstream ss;
	ss << msg << " at row " << it->_line + 1 << ", column " << it->_offset + 1;
	throw std::runtime_error(RBOLD(ss.str()));
}

void Config::printProgressBar(size_t progress, size_t total)
{
    const int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress / total;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0 / total) << " %\r";
    std::cout.flush();
}
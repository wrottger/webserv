#include "Config.hpp"
#include "colors.hpp"

Config::Config(void) : _isLoaded(false)
{

}

bool Config::isLoaded(void) const
{
    return _isLoaded;
}

const std::vector<Config::Node>& Config::getNodes(void) const
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

std::vector<Config::ServerBlock>& Config::getServerBlocks(void)
{
    return _serverBlocks;
}

std::vector<int> Config::getPorts(std::vector<ServerBlock>& _serverBlocks) 
{
    std::vector<int> ports;

    for (std::vector<ServerBlock>::iterator it = _serverBlocks.begin(); it != _serverBlocks.end(); it++)
    {
        for (std::vector<std::pair<TokenType, std::string> >::iterator it2 = it->_directives.begin(); it2 != it->_directives.end(); it2++)
        {
            if (it2->first == Port)
            {
                std::stringstream ss(it2->second);
                int temp;
                ss >> temp;
                if (std::find(ports.begin(), ports.end(), temp) == ports.end())
                    ports.push_back(temp);
            }
        }
    }
return ports;
}

struct position
{
    size_t serverBlock;
    size_t locationBlock;
};

// returns the index of the server block and location block that best matches the given route and host
std::pair<size_t, size_t> Config::getClosestPathMatch(std::string route, std::string host)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use getClosestPathMatch without a valid Config instance.");
    std::vector<std::pair<std::string, position> > paths;
    for (size_t s_count = 0; s_count != config->_serverBlocks.size(); s_count++) //iterate through server blocks
    {
        for (size_t d = 0; d != config->_serverBlocks[s_count]._directives.size(); d++) //iterate through location blocks
        {
            if (config->_serverBlocks[s_count]._directives[d].first == ServerName && config->_serverBlocks[s_count]._directives[d].second == host)
            {
                for (size_t l_count = 0; l_count != config->_serverBlocks[s_count]._locations.size(); l_count++)
                {
                    if (route.find(config->_serverBlocks[s_count]._locations[l_count]._path) == 0)
                    {
                        position position;
                        position.serverBlock = s_count;
                        position.locationBlock = l_count;
                        paths.push_back(std::make_pair(config->_serverBlocks[s_count]._locations[l_count]._path, position));
                    }
                }
            }
        }
    }
    if (paths.size() == 0)
        return std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());

    std::pair<std::string, position > temp = paths[0];
    for (size_t i = 0; i != paths.size(); i++)
    {
        if (paths[i].first.size() > temp.first.size())
            temp = paths[i];
    }
    return std::make_pair(temp.second.serverBlock, temp.second.locationBlock);
}

// returns true if the directive is allowed for the given route, host, and value
bool Config::isDirectiveAllowed(const std::string& route, const std::string& host, const Config::TokenType directive, const std::string& value)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use isDirectiveAllowed without a valid Config instance.");
    std::pair<size_t, size_t> l = config->getClosestPathMatch(route, host);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return false;
    for (size_t i = 0; i != config->_serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config->_serverBlocks[l.first]._locations[l.second]._directives[i].first == directive
            && config->_serverBlocks[l.first]._locations[l.second]._directives[i].second.find(value) != std::string::npos) // check if the directive is allowed
            return true;
    }
    return false;
}

// returns the root directory for the given route and host or empty string if not found (location block directives take precedence over server block directives)
std::string Config::getRootDirectory(const std::string route, const std::string host)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use getRootDirectory without a valid Config instance.");
    std::pair<size_t, size_t> l = config->getClosestPathMatch(route, host);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return "";
    for (size_t i = 0; i != config->_serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config->_serverBlocks[l.first]._locations[l.second]._directives[i].first == Root)
            return config->_serverBlocks[l.first]._locations[l.second]._directives[i].second;
    }
    for (size_t i = 0; i != config->_serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config->_serverBlocks[l.first]._directives[i].first == Root)
            return config->_serverBlocks[l.first]._directives[i].second;
    }
    return "";
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
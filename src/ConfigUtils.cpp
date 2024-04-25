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

// returns the location block that is the closest match for the given path; returns NULL if no match is found
Config::LocationBlock* Config::getClosestPathMatch(std::string path, std::string host)
{
    if (getInstance() == NULL)
        throw std::runtime_error("Cannot use getClosestLocationBlock without a valid Config instance.");

    Config* config = getInstance();
    std::vector<std::pair<std::string, LocationBlock> > paths;
    for (size_t s_count = 0; s_count != config->_serverBlocks.size(); s_count++) //iterate through server blocks
    {
        for (size_t d = 0; d != config->_serverBlocks[s_count]._directives.size(); d++) //iterate through location blocks
        {
            if (config->_serverBlocks[s_count]._directives[d].first == ServerName && config->_serverBlocks[s_count]._directives[d].second == host)
            {
                for (size_t l = 0; l != config->_serverBlocks[s_count]._locations.size(); l++)
                {
                    if (path.find(config->_serverBlocks[s_count]._locations[l]._path) == 0)
                    {
                        paths.push_back(std::make_pair(config->_serverBlocks[s_count]._locations[l]._path, config->_serverBlocks[s_count]._locations[l]));
                    }
                }
            }
        }
    }
    if (paths.size() == 0)
        return static_cast<LocationBlock*>(NULL);
    
    std::pair<std::string, LocationBlock> temp = paths[0];
    for (size_t i = 0; i != paths.size(); i++)
    {
        if (paths[i].first.size() > temp.first.size())
            temp = paths[i];
    }
    LocationBlock *ret = new LocationBlock(temp.second); // dont forget to delete this
    return ret;
}

// isMethodAllowed(string -> Path, string -> Method)
// isCgiAllowed(string -> Path, string -> Fileextension)
// getRootDirectory(string -> Path, string -> Host)


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
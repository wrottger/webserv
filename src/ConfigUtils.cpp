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
    for (size_t server = 0; server != config->_serverBlocks.size(); server++) //iterate through server blocks
    {
        for (size_t directive = 0; directive != config->_serverBlocks[server]._directives.size(); directive++) //iterate through location blocks
        {
            if (config->_serverBlocks[server]._directives[directive].first == ServerName && config->_serverBlocks[server]._directives[directive].second == host)
            {
                for (size_t location = 0; location != config->_serverBlocks[server]._locations.size(); location++)
                {
                    std::string locationPath = config->_serverBlocks[server]._locations[location]._path;

                    // Add trailing slashes to route and locationPath if they don't have one
                    if (!route.empty() && route[route.size() - 1] != '/')
                        route += '/';
                    if (!locationPath.empty() && locationPath[locationPath.size() - 1] != '/')
                        locationPath += '/';

                    if (route.find(locationPath) == 0)
                    {
                        position position;
                        position.serverBlock = server;
                        position.locationBlock = location;
                        paths.push_back(std::make_pair(locationPath, position));
                    }
                }
            }
        }
    }
    if (paths.size() == 0)
        return std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max()); // max value on failure

    std::pair<std::string, position > temp = paths[0];
    for (size_t i = 0; i != paths.size(); i++)
    {
        if (paths[i].first.size() > temp.first.size())
            temp = paths[i];
    }
    std::cout << "temp.first: " << temp.first << std::endl;
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

// returns the value of the directive for the given route, host, and directive (shouldn't be used for directives with special cases like root)
std::string Config::getDirectiveValue(const std::string& route, const std::string& host, const Config::TokenType directive)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use getDirectiveValue without a valid Config instance.");
    std::pair<size_t, size_t> l = config->getClosestPathMatch(route, host);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return "";
    for (size_t i = 0; i != config->_serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config->_serverBlocks[l.first]._locations[l.second]._directives[i].first == directive)
            return config->_serverBlocks[l.first]._locations[l.second]._directives[i].second;
    }
    for (size_t i = 0; i != config->_serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config->_serverBlocks[l.first]._directives[i].first == directive)
            return config->_serverBlocks[l.first]._directives[i].second;
    }
    return "";
}

bool Config::isValidPath(const std::string& path)
{
    std::string specialChars = "*?|<>:\"~\t ";
    std::string controlChars = "";

    for (char c = 0; c < 32; ++c)
    {
        controlChars += c;
    }
    controlChars += 127;

    if (path[0] != '/') // check if the path is absolute
        return false;
    else if (path.find("/.") != std::string::npos) // prevent hidden files
        return false;
    else if (path.find("..") != std::string::npos) // prevent directory traversal
        return false;
    else if (path.find("//") != std::string::npos)
        return false;
    else if (path.find_first_of(specialChars) != std::string::npos) // check for special characters
        return false;
    else if (path.find_first_of(controlChars) != std::string::npos) // check for control characters
        return false;
    return true;
}
// returns the root directory for the given route and host or empty string if not found (location block directives take precedence over server block directives)
std::string Config::getRootDirectory(const std::string route, const std::string host)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use getRootDirectory without a valid Config instance.");
    std::pair<size_t, size_t> position = config->getClosestPathMatch(route, host); // first = server block index, second = location block index
    std::cout << "position.first: " << position.first << " position.second: " << position.second << std::endl;
    if (position.first == std::numeric_limits<size_t>::max() || position.second == std::numeric_limits<size_t>::max())
        return "";
    for (size_t i = 0; i != config->_serverBlocks[position.first]._locations[position.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config->_serverBlocks[position.first]._locations[position.second]._directives[i].first == Root)
            return config->_serverBlocks[position.first]._locations[position.second]._directives[i].second;
    }
    for (size_t i = 0; i != config->_serverBlocks[position.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config->_serverBlocks[position.first]._directives[i].first == Root)
            return config->_serverBlocks[position.first]._directives[i].second;
    }
    return "";
}

std::string Config::getFilePath(const std::string filePath, const std::string host)
{
    Config* config = getInstance();
    if (config == NULL) //prevent segfault
        throw std::runtime_error("Cannot use getFilePath without a valid Config instance.");
    
    std::string root = config->getRootDirectory(filePath, host);
    if (root.empty())
        throw std::runtime_error("No root directory found for filePath: " + filePath + ", host: " + host);

    // Ensure root does not end with a slash
    if (!root.empty() && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    // Ensure filePath does not start with a slash
    std::string normalizedFilePath = filePath;
    if (!normalizedFilePath.empty() && normalizedFilePath[0] == '/')
        normalizedFilePath = normalizedFilePath.substr(1);

    // Concatenate root and normalizedFilePath
    std::string result = root + "/" + normalizedFilePath;

    return result;
}

std::string Config::getDir(const std::string filePath, const std::string host)
{
    std::string path = getFilePath(filePath, host);
    return path.substr(0, path.find_last_of("/"));
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
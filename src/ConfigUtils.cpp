#include "Config.hpp"
#include "colors.hpp"
#include "Logger.hpp"

Config::Config(void) : _isLoaded(false)
{

}

bool Config::isLoaded(void) const
{
    return _isLoaded;
}

bool Config::isMethodAllowed(const HttpHeader &header, const std::string &method) const {
	return Config::getInstance().isDirectiveAllowed(header, Config::AllowedMethods, method);
}

const std::vector<Config::Node>& Config::getNodes(void) const
{
    return _nodes;
}

bool Config::compareNodes(const Node& a, const Node& b)
{
    return a._line < b._line || (a._line == b._line && a._offset < b._offset);
}

Config &Config::getInstance()
{
	static Config instance;
	return instance;
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

struct pathInfo
{
    std::string _path;
    size_t _serverBlock;
    size_t _locationBlock;
};

// returns the index of the server block and location block that best matches the given route and host
std::pair<size_t, size_t> Config::getClosestPathMatch(const HttpHeader& header)
{
    Config &config = getInstance();
    std::vector<pathInfo> paths;
    std::string route = header.getPath();
    std::string host = header.getHost();

    std::stringstream ss;
    ss << header.getPort();
    std::string port = ss.str();

    // server block matching
    size_t serverMatch = std::numeric_limits<size_t>::max();
    for (size_t server = 0; server != config._serverBlocks.size(); server++) //iterate through server blocks
    {
        bool portSet = false;
        bool hostSet = false;
        for (size_t directive = 0; directive != config._serverBlocks[server]._directives.size(); directive++) //iterate through location blocks
        {
            if (config._serverBlocks[server]._directives[directive].first == Port && config._serverBlocks[server]._directives[directive].second == port)
            {
                portSet = true;
                // Store the first port match
                if (serverMatch == std::numeric_limits<size_t>::max())
                    serverMatch = server; // use this server block if no host match is found later
            }
            if (config._serverBlocks[server]._directives[directive].first == ServerName && config._serverBlocks[server]._directives[directive].second == host)
                hostSet = true; 
        }
        if (portSet && hostSet)
        {
            serverMatch = server; // if both port and host match, use this server block else continue searching
            break;
        }
    }
    // location block matching
    for (size_t location = 0; location != config._serverBlocks[serverMatch]._locations.size(); location++)
    {
        std::string locationPath = config._serverBlocks[serverMatch]._locations[location]._path;
        // Add trailing slashes to route and locationPath if they don't have one
        if (!route.empty() && route[route.size() - 1] != '/')
            route += '/';
        if (!locationPath.empty() && locationPath[locationPath.size() - 1] != '/')
            locationPath += '/';
        if (route.find(locationPath) == 0)
        {
            pathInfo match;
            match._serverBlock = serverMatch;
            match._locationBlock = location;
            match._path = locationPath;
            paths.push_back(match);
        }
    }
    if (paths.size() == 0)
        return std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max()); // max value on failure

    pathInfo temp = paths[0];
    for (size_t i = 0; i != paths.size(); i++)
    {
        if (paths[i]._path.size() > temp._path.size())
            temp = paths[i];
    }
    return std::make_pair(temp._serverBlock, temp._locationBlock);
}

std::pair<size_t, size_t> Config::getClosestPathMatch(std::string& route, const HttpHeader& header)
{
    Config &config = getInstance();
    std::vector<pathInfo> paths;
    std::string host = header.getHost();

    std::stringstream ss;
    ss << header.getPort();
    std::string port = ss.str();

    // server block matching
    size_t serverMatch = std::numeric_limits<size_t>::max();
    for (size_t server = 0; server != config._serverBlocks.size(); server++) //iterate through server blocks
    {
        bool portSet = false;
        bool hostSet = false;
        for (size_t directive = 0; directive != config._serverBlocks[server]._directives.size(); directive++) //iterate through location blocks
        {
            if (config._serverBlocks[server]._directives[directive].first == Port && config._serverBlocks[server]._directives[directive].second == port)
            {
                portSet = true;
                // Store the first port match
                if (serverMatch == std::numeric_limits<size_t>::max())
                    serverMatch = server; // use this server block if no host match is found later
            }
            if (config._serverBlocks[server]._directives[directive].first == ServerName && config._serverBlocks[server]._directives[directive].second == host)
                hostSet = true; 
        }
        if (portSet && hostSet)
        {
            serverMatch = server; // if both port and host match, use this server block else continue searching
            break;
        }
    }
    // location block matching
    for (size_t location = 0; location != config._serverBlocks[serverMatch]._locations.size(); location++)
    {
        std::string locationPath = config._serverBlocks[serverMatch]._locations[location]._path;
        // Add trailing slashes to route and locationPath if they don't have one
        if (!route.empty() && route[route.size() - 1] != '/')
            route += '/';
        if (!locationPath.empty() && locationPath[locationPath.size() - 1] != '/')
            locationPath += '/';
        if (route.find(locationPath) == 0)
        {
            pathInfo match;
            match._serverBlock = serverMatch;
            match._locationBlock = location;
            match._path = locationPath;
            paths.push_back(match);
        }
    }
    if (paths.size() == 0)
        return std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max()); // max value on failure

    pathInfo temp = paths[0];
    for (size_t i = 0; i != paths.size(); i++)
    {
        if (paths[i]._path.size() > temp._path.size())
            temp = paths[i];
    }
    return std::make_pair(temp._serverBlock, temp._locationBlock);
}

// returns true if the directive is allowed for the given route, host, and value
bool Config::isDirectiveAllowed(const HttpHeader& header, const Config::TokenType directive, const std::string& value)
{
    Config &config = getInstance();
    std::pair<size_t, size_t> l = config.getClosestPathMatch(header);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return false;
    for (size_t i = 0; i != config._serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config._serverBlocks[l.first]._locations[l.second]._directives[i].first == directive
            && config._serverBlocks[l.first]._locations[l.second]._directives[i].second.find(value) != std::string::npos) // check if the directive is allowed
            return true;
    }
    return false;
}

// returns the value of the directive for the given route, host, and directive (priority is given to location block directives)
std::string Config::getDirectiveValue(const HttpHeader& header, const Config::TokenType directive)
{
    Config &config = getInstance();
    std::pair<size_t, size_t> l = config.getClosestPathMatch(header);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return "";
    for (size_t i = 0; i != config._serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config._serverBlocks[l.first]._locations[l.second]._directives[i].first == directive)
            return config._serverBlocks[l.first]._locations[l.second]._directives[i].second;
    }
    for (size_t i = 0; i != config._serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config._serverBlocks[l.first]._directives[i].first == directive)
            return config._serverBlocks[l.first]._directives[i].second;
    }
    return "";
}

std::string Config::getErrorPage(int code, const HttpHeader& header)
{
    Config &config = getInstance();
    std::pair<size_t, size_t> l = config.getClosestPathMatch(header);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max())
        return "";
    std::stringstream ss;
    ss << code;
    std::string codeStr = ss.str();
    for (size_t i = 0; i != config._serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config._serverBlocks[l.first]._locations[l.second]._directives[i].first == ErrorPage)
        {
            size_t pos = config._serverBlocks[l.first]._locations[l.second]._directives[i].second.find(' ');
            if (codeStr == config._serverBlocks[l.first]._locations[l.second]._directives[i].second.substr(0, pos))
                return config._serverBlocks[l.first]._locations[l.second]._directives[i].second.substr(pos + 1);
        }
    }
    for (size_t i = 0; i != config._serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config._serverBlocks[l.first]._directives[i].first == ErrorPage)
        {
            size_t pos = config._serverBlocks[l.first]._directives[i].second.find(' ');
            if (codeStr == config._serverBlocks[l.first]._directives[i].second.substr(0, pos))
                return config._serverBlocks[l.first]._directives[i].second.substr(pos + 1);
        }
    }
    return "";

}

std::string Config::getCgiScriptPath(const HttpHeader &header)
{
    std::string path;
    bool isFile = 0;
    std::istringstream iss(header.getPath());
    // get the extension of the file
    for (std::string token; std::getline(iss, token, '/');)
    {
        path += token;
        if (token.find('.') != std::string::npos && token.find('.') != token.size() - 1)
        {
            isFile = 1;
            break;
        }
		path += "/";
    }
    if (!isFile)
        return "";
	else if (!path.size())
		path = "/";
    std::string absolutePath = getFilePath(header, path);
    return absolutePath;
}

std::string Config::getCgiDir(const HttpHeader &header)
{
    std::string path = "";
    int file = 0;
    std::istringstream iss(header.getPath());
    // get the extension of the file
    for (std::string token; std::getline(iss, token, '/');)
    {
        path += token;
        if (token.find('.') != std::string::npos && token.find('.') != token.size() - 1)
        {
            file = 1;
            break;
        }
		path += "/";
    }
    if (!file)
        return "";
	else if (!path.size())
		path = "/";
    std::string absolutePath = getDir(header);
    return absolutePath;
}

std::string Config::getCgiInterpreterPath(const HttpHeader &header) {
	Config &config = getInstance();
    std::string path = "";
    std::string extension;
    std::istringstream iss(header.getPath());
    // get the extension of the file
    for (std::string token; std::getline(iss, token, '/');)
    {
        path += token;
        if (token.find('.') != std::string::npos && token.find('.') != token.size() - 1)
        {
            extension = token.substr(token.find_last_of('.'));
            break;
        }
		path += "/";
    }
	if(!path.size())
		path = "/";
    std::pair<size_t, size_t> l = config.getClosestPathMatch(path, header);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max() || extension.empty())
        return "";
    // check if the extension is allowed in the location CGI directive
    for (size_t i = 0; i != config._serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config._serverBlocks[l.first]._locations[l.second]._directives[i].first == CGI)
        {
            std::string value = config._serverBlocks[l.first]._locations[l.second]._directives[i].second;
            size_t pos = value.find(' ');
            if (extension == value.substr(0, pos))
                return value.substr(pos + 1);
        }
    }
    // check if the extension is allowed in the server CGI directive
    for (size_t i = 0; i != config._serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config._serverBlocks[l.first]._directives[i].first == CGI)
        {
            std::string value = config._serverBlocks[l.first]._directives[i].second;
            size_t pos = value.find(' ');
            if (extension == value.substr(0, pos))
                return value.substr(pos + 1);

        }
    }
    return "";
}

bool Config::isCGIAllowed(const HttpHeader &header)
{
	if (header.getMethod() == "DELETE"){
		return false;
	}
    Config &config = getInstance();
    std::string path = "";
    std::string extension;
    std::istringstream iss(header.getPath());
    // get the extension of the file
    for (std::string token; std::getline(iss, token, '/');)
    {
        path += token;
        if (token.find('.') != std::string::npos && token.find('.') != token.size() - 1)
        {
            extension = token.substr(token.find_last_of('.'));
            break;
        }
		path += "/";
    }
	if(!path.size())
		path = "/";
    std::pair<size_t, size_t> l = config.getClosestPathMatch(path, header);
    if (l.first == std::numeric_limits<size_t>::max() || l.second == std::numeric_limits<size_t>::max() || extension.empty())
        return false;
    // check if the extension is allowed in the location CGI directive
    for (size_t i = 0; i != config._serverBlocks[l.first]._locations[l.second]._directives.size(); i++) // iterate through location block directives
    {
        if (config._serverBlocks[l.first]._locations[l.second]._directives[i].first == CGI)
        {
            std::stringstream ss(config._serverBlocks[l.first]._locations[l.second]._directives[i].second);
            for (std::string token; std::getline(ss, token, ' ');)
            {
                if (token == extension)
                    return true;
            }
        }
    }
    // check if the extension is allowed in the server CGI directive
    for (size_t i = 0; i != config._serverBlocks[l.first]._directives.size(); i++) // iterate through server block directives
    {
        if (config._serverBlocks[l.first]._directives[i].first == CGI)
        {
            std::stringstream ss(config._serverBlocks[l.first]._directives[i].second);
            for (std::string token; std::getline(ss, token, ' ');)
            {
                if (token == extension)
                    return true;
            }
        }
    }
    return false;
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

//returns true the first time a host is set with the given port and false otherwise
bool Config::isHostSet(const HttpHeader& header)
{
    Config &config = getInstance();
    std::stringstream ss;
    ss << header.getPort();
    std::string port = ss.str();
    for (size_t i = 0; i != config._serverBlocks.size(); i++)
    {
        for (size_t j = 0; j != config._serverBlocks[i]._directives.size(); j++)
        {
            if (config._serverBlocks[i]._directives[j].first == ServerName && config._serverBlocks[i]._directives[j].second == header.getHost())
            {
                for (size_t k = 0; k != config._serverBlocks[i]._directives.size(); k++)
                {
                    if (config._serverBlocks[i]._directives[k].first == Port && config._serverBlocks[i]._directives[k].second == port)
                        return true;
                }
            }
        }
    }
    return false;
}

std::string Config::getFilePath(const HttpHeader& header)
{
    Config &config = getInstance();
    std::string root = config.getDirectiveValue(header, Root);
    if (root.empty())
        return "";

    // Ensure root does not end with a slash
    if (!root.empty() && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    // Ensure filePath does not start with a slash
    std::string normalizedFilePath = header.getPath();
    if (!normalizedFilePath.empty() && normalizedFilePath[0] == '/')
        normalizedFilePath = normalizedFilePath.substr(1);

    // Concatenate root and normalizedFilePath
    std::string result = root + "/" + normalizedFilePath;

    return result;
}

std::string Config::getFilePath(const HttpHeader &header, std::string path)
{
    Config &config = getInstance();
    std::string root = config.getDirectiveValue(header, Root);
    if (root.empty())
        return "";

    // Ensure root does not end with a slash
    if (!root.empty() && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    // Ensure filePath does not start with a slash
    std::string normalizedFilePath = path;
    if (!normalizedFilePath.empty() && normalizedFilePath[0] == '/')
        normalizedFilePath = normalizedFilePath.substr(1);

    // Concatenate root and normalizedFilePath
    std::string result = root + "/" + normalizedFilePath;

    return result;
}

std::string Config::getDir(const HttpHeader &header) {
	std::string path = getFilePath(header);
    return path.substr(0, path.find_last_of("/"));
}

size_t Config::getMaxBodySize(const HttpHeader &header) {
	return Utils::stringToNumber(getDirectiveValue(header, ClientMaxBodySize));
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

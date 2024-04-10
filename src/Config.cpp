#include "Config.hpp"
#include "colors.hpp"
#include "tokens.hpp"

Config::Config(std::string filename) : _fileName(filename), _isLoaded(false)
{
    std::cout << GBOLD("Config created") << std::endl;
}

Config::Config(const Config &src)
{
    std::cout << GBOLD("Config copied") << std::endl;
    _tokens = src._tokens;
    _fileName = src._fileName;
    _isLoaded = src._isLoaded;
    _fileContent = src._fileContent;
}

Config &Config::operator=(const Config &src)
{
    std::cout << GBOLD("Config assigned") << std::endl;
    if (this != &src)
    {
        _tokens = src._tokens;
        _fileName = src._fileName;
        _isLoaded = src._isLoaded;
        _fileContent = src._fileContent;
    }
    return *this;
}

Config::~Config()
{
    std::cout << GBOLD("Config deleted") << std::endl;
}

std::vector<std::string> Config::getFileContent(void)
{
    return _fileContent;
}

std::string Config::getFileName(void) const
{
    return _fileName;
}

bool Config::isLoaded(void) const
{
    return _isLoaded;
}

std::ostream &operator<<(std::ostream &os, const Config &src)
{
    os << "Config file: " << src.getFileName() << std::endl;
    return os;
}

void Config::openConfigFile(std::string filename)
{
    std::ifstream _fileStream(filename.c_str());
    if (!_fileStream.is_open())
    {
        throw std::runtime_error(RBOLD("Error: Unable to open file " + filename));
    }
    while (!_fileStream.eof())
    {
        std::string line;
        std::getline(_fileStream, line);
        _fileContent.push_back(line);
    }
    _tokens["server"] = Server;
    _tokens["location"] = Location;
    _tokens["port"] = Port;
    _tokens["host"] = Host;
    _tokens["root"] = Root;
    _tokens["listing"] = Listing;
    _tokens["index"] = Index;
    _tokens["redir"] = Redir;
    _tokens["server_name"] = ServerName;
    _tokens["allowed_methods"] = AllowedMethods;
    _tokens["client_max_body_size"] = ClientMaxBodySize;
    _tokens["default"] = Default;
    _tokens["cgi"] = CGI;
    _tokens["{"] = OpenBrace;
    _tokens["}"] = CloseBrace;
    _tokens[";"] = Semicolon;
    return ;
}

//recursive function to parse the config file
void Config::tokenizeConfigFile(std::vector<std::string>::iterator it)
{

    return ;
}  

TokenType Config::getNextToken(std::string::iterator it, std::string::iterator end)
{
    for (; it != end; it++)

}

void Config::checkScopes(void)
{
    size_t scopeLevel = 0;
    std::vector<std::pair<size_t, size_t> > scopes; // pair of start and end of scope
    for (std::vector<std::string>::iterator it = _fileContent.begin(); it != _fileContent.end(); it++)
    {
        if (it->find("{") != std::string::npos)
        {
            scopes.push_back(std::make_pair(it - _fileContent.begin(), 0));
            scopeLevel++;
        }
        else if (it->find("}") != std::string::npos)
        {
            if (scopeLevel == 0)
                std::cout << RBOLD("Syntax error: unexpected '}' at row ") << it - _fileContent.begin() + 1 << RBOLD(", column ") << it->find("}") + 1 << std::endl;
            else
            {
                scopes[scopeLevel - 1].second = it - _fileContent.begin();
                scopeLevel--;
            }
        }
    }
    for (std::vector<std::pair<size_t, size_t> >::iterator it = scopes.begin(); it != scopes.end(); it++)
    {
        if (it->second == 0)
            std::cout << RBOLD("Syntax error: missing '}' at row ") << it->first + 1 << RBOLD(", column ") << _fileContent[it->first].find("{") + 1 << std::endl;
    }
}
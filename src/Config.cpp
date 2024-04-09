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
    _configMap = src._configMap;
    _fileName = src._fileName;
    _isLoaded = src._isLoaded;
    _fileContent = src._fileContent;
}

Config &Config::operator=(const Config &src)
{
    std::cout << GBOLD("Config assigned") << std::endl;
    if (this != &src)
    {
        _configMap = src._configMap;
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
    return ;
}

void Config::tokenizeConfigFile(void)
{
    std::vector<Token> tokens;
    for (std::vector<std::string>::iterator it = _fileContent.begin(); it != _fileContent.end(); it++)
    {
        // skip whitespaces
        size_t pos = it->find_first_not_of(" \t");
        std::string::iterator it2 = it->begin() + pos;
        for (int i = 0; i < 2; i++)
        {
            if (std::string(it2, it2 + std::string(tokenTypes[i]).length()) == tokenTypes[i])
            {
                tokens.push_back(Token(tokenTypes[i], pos));
            }
        }
    }
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        std::cout << it->_type << " " << it->_value << " " << it->_offset << std::endl;
    }
    return ;
}  

void Config::parseConfigFile(void)
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
#include "Config.hpp"
#include "colors.hpp"

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

void Config::openConfigFile(std::string filename)
{
    std::ifstream _fileStream(filename.c_str());
    if (!_fileStream.is_open())
    {
        throw std::runtime_error("Error: Unable to open file " + filename);
    }
    while (!_fileStream.eof())
    {
        std::string line;
        std::getline(_fileStream, line);
        _fileContent.push_back(line);
    }
    return ;
}

std::vector<std::string> Config::getFileContent(void)
{
    return _fileContent;
}

std::string Config::getFileName(void) const
{
    return _fileName;
}

std::ostream &operator<<(std::ostream &os, const Config &src)
{
    os << "Config file: " << src.getFileName() << std::endl;
    return os;
}

void Config::parseConfigFile(void)
{
    for (std::vector<std::string>::iterator it = _fileContent.begin(); it != _fileContent.end(); it++)
    {
        it->
    }
}
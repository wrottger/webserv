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
    _fileName = src._fileName;
}

Config &Config::operator=(const Config &src)
{
    std::cout << GBOLD("Config assigned") << std::endl;
    if (this != &src)
    {
        _configMap = src._configMap;
        _fileName = src._fileName;
        _isLoaded = src._isLoaded;
        _fileName = src._fileName;
    }
    return *this;
}

Config::~Config()
{
    std::cout << GBOLD("Config deleted") << std::endl;
    _fileStream.close();
}

void Config::openConfigFile(std::string filename)
{
	_fileStream.open(filename.c_str(), std::ios::in);
	if (!_fileStream.is_open())
        throw std::runtime_error("Error: \"" + filename + "\" could not be opened");
    return ;
}

std::string Config::getFileName(void) const
{
    return _fileName;
}

std::fstream &Config::getStream(void)
{
    return _fileStream;
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

void Config::parseConfigFile(void)
{
    size_t row = 1;
    for (std::string line; std::getline(getStream(), line, '\n'); row++)
    {
        if (line.empty())
            throw Config::ConfigException("Empty line", row, 0);
        std::cout << line << std::endl;
    }
}
#include "Config.hpp"

Config::Config(std::fstream &file)
{
    
}

Config::Config(const Config &src)
{
    if (this != &src)
    {
        _configMap = src._configMap;
    }
}

Config::~Config()
{

}

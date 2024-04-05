#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP
# include <map>
# include <string>
# include <fstream>
# include <iostream>

class ConfigParser {

    private:

        std::map<std::string, std::string> configMap;
        const std::string configFilePath;

    public:

        ConfigParser(std::string configFilePath);

};

#endif
#ifndef CONFIG_HPP
#define CONFIG_HPP
# include <map>
# include <string>
# include <fstream>
# include <iostream>

class Config {

    private:

        std::map<std::string, std::string> _configMap;
        const std::string _configFile;

    public:

        Config(std::fstream& file);
        Config(const Config& src);
        Config& operator=(const Config& src);
        ~Config();

};

#endif
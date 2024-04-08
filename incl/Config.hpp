#ifndef CONFIG_HPP
#define CONFIG_HPP
# include <map>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>

class Config {

    private:

        std::map<std::string, std::string> _configMap;
        std::string _fileName;
        std::fstream _fileStream;
        bool _isLoaded;

        Config(const Config& src);  // private copy constructor and assignment operator to prevent dangling references/pointers
        Config& operator=(const Config& src); 

    public:

        Config(std::string filename);
        ~Config();

        void openConfigFile(std::string filename);
        std::string getFileName(void) const;
        std::fstream& getStream(void);
        bool isLoaded(void) const;
        void parseConfigFile(void);

        class ConfigException : public std::exception {
            private:

                int _row;
                int _column;
                std::string _msg;

            public:
                ConfigException(std::string msg, int row, int column) : _row(row), _column(column), _msg(msg)  {}
                virtual ~ConfigException() throw() {}
                virtual const char* what() const throw() {
                    return _msg.c_str();
                }
                int getRow(void) const { return _row; }
                int getColumn(void) const { return _column; }
        };
};

std::ostream& operator<<(std::ostream& os, const Config& src);

#endif
#ifndef CONFIG_HPP
#define CONFIG_HPP
# include "tokens.hpp"
# include <map>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>

class Config {

    private:

        std::string _fileName;
        std::vector<std::string> _fileContent;
        bool _isLoaded;

        Config(const Config& src);  // private copy constructor and assignment operator to prevent dangling references/pointers
        Config& operator=(const Config& src); 

    public:

        Config(std::string filename);
        ~Config();

        // getters
        std::string getFileName(void) const;
        std::vector<std::string> getFileContent(void);
        bool isLoaded(void) const;

        // config parsing methods
        void openConfigFile(std::string filename);
        void scanTokens(std::vector<std::string> fileContent);
        void checkScopes(void);
        std::vector<std::string> slice(std::string in, std::vector<char> delim);
        TokenType getNextToken(std::string::iterator it, std::string::iterator end);

        // token map
        std::map<std::string, TokenType> _tokens;

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
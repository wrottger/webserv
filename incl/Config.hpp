#ifndef CONFIG_HPP
#define CONFIG_HPP
# include "tokens.hpp"
# include <map>
# include <stack>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>

class Config {

    private:
        std::vector<Node> _nodes;
        bool _isLoaded;

        Config(const Config& src);  // private copy constructor and assignment operator to prevent dangling references/pointers
        Config& operator=(const Config& src); 

    public:

        Config();
        ~Config();

        // getters
        const std::vector<Node>& getNodes(void) const;
        bool isLoaded(void) const;

        // config parsing methods
        void parseConfigFile(std::string filename);
        void checkScopes(void);
        void scanTokens(std::ifstream& file);
        std::vector<std::pair<std::string, size_t> > slice(std::string in, std::vector<char> delim);
        TokenType getNextToken(std::string::iterator it, std::string::iterator end);

        // utils
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

#endif
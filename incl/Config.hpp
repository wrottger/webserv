#ifndef CONFIG_HPP
#define CONFIG_HPP
# include "Tokens.hpp"
# include "ServerBlock.hpp"
# include <map>
# include <stack>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>
# include <cstdlib>

class Config {

    private:
        std::vector<Node> _nodes;
        std::vector<ServerBlock> _serverBlocks;
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
        void scanTokens(std::ifstream& file);
        void parseScopes(void);
        void buildAST(std::vector<Node>::iterator it, std::vector<Node>::iterator end);
        void parseTokens(void);
        void error(const std::string &msg, const std::vector<Node>::iterator& it);
        void sortVector(std::vector<Node>& vec);
        void addServerBlock(ServerBlock& newBlock, std::vector<Node>::iterator& start);
        ServerBlock parseServerBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end);
        LocationBlock parseLocationBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end);
        std::vector<std::pair<std::string, size_t> > slice(std::string in, std::vector<char> delim);

        // utils
        std::map<std::string, TokenType> _tokens;
};

#endif
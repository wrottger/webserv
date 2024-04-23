#ifndef CONFIG_HPP
#define CONFIG_HPP
# include "Nodes.hpp"
# include "ServerBlock.hpp"
# include <map>
# include <stack>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>
# include <cstdlib>
# include <algorithm>

class Config {

    private:

        Config();
        Config& operator=(const Config& src); 
        Config(const Config& src);
        static Config* _instance;

        std::vector<Node> _nodes;
        std::vector<ServerBlock> _serverBlocks;
        size_t _lines;
        bool _isLoaded;


    public:
        // getters
        static Config* getInstance();
        const std::vector<Node>& getNodes(void) const;
        std::vector<ServerBlock>& getServerBlocks(void);
        bool isLoaded(void) const;
        static std::vector<int> getPorts(std::vector<ServerBlock>& _serverBlocks);

        // config parsing methods
        void parseConfigFile(std::string filename);
        void scanTokens(std::ifstream& file);
        void parseScopes(void);
        void buildAST(std::vector<Node>::iterator it, std::vector<Node>::iterator end);
        void parseTokens(void);
        void error(const std::string &msg, const std::vector<Node>::iterator& it);
        void sortVector(std::vector<Node>& vec);
        void addServerBlock(ServerBlock& newBlock, std::vector<Node>::iterator& start);
        void printProgressBar(size_t progress, size_t total);
        ServerBlock parseServerBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end);
        LocationBlock parseLocationBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end);
        std::vector<std::pair<std::string, size_t> > slice(std::string in, std::vector<char> delim);

        // utils
        std::map<std::string, TokenType> _tokens;
};

#endif

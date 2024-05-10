#ifndef CONFIG_HPP
#define CONFIG_HPP
# include <map>
# include <stack>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>
# include <sstream>
# include <cstdlib>
# include <algorithm>
# include <limits>

class Config {

    public:

        // structs
        struct TokenInfo {
            std::string token;
            size_t position;
        };
        struct ServerBlock;
        struct Node;
        struct LocationBlock;
        enum TokenType
        {
            OpenBrace,
            CloseBrace,
            Server,
            Location,
            Listing,
            Port,
            Root,
            Index,
            Redir,
            ServerName,
            AllowedMethods,
            ClientMaxBodySize,
            Default,
            CGI,
            Data,
            Semicolon,
            ErrorPage,
        };

        std::map<std::string, TokenType> _tokens;
        
        // getters
        static Config* getInstance();
        static std::vector<int> getPorts(std::vector<ServerBlock>& _serverBlocks);
        const std::vector<Node>& getNodes(void) const;
        std::vector<ServerBlock>& getServerBlocks(void);
        bool isLoaded(void) const;
        bool isValidPath(const std::string& path);
        bool isDirectiveAllowed(const std::string& route, const std::string& host, const Config::TokenType directive, const std::string& value);
        bool isHostSet(const std::string& host, const std::string& port);
        std::string getCgiDir(const std::string& route, const std::string& host);
        std::string getCgiScriptPath(const std::string& route, const std::string& host);
        bool isCGIAllowed(const std::string& route, const std::string& host);
        std::pair<size_t, size_t> getClosestPathMatch(std::string route, std::string host);
        std::string getErrorPage(int code, const std::string& route, const std::string& host);
        std::string getDirectiveValue(const std::string& route, const std::string& host, const Config::TokenType directive);
        std::string getRootDirectory(const std::string route, const std::string host);
        std::string getFilePath(const std::string filePath, const std::string host);
        std::string getDir(const std::string filePath, const std::string host);

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
        std::vector<TokenInfo> slice(std::string in, std::vector<char> delim);

    private:

        Config();
        Config& operator=(const Config& src); 
        Config(const Config& src);
        static Config* _instance;

        std::vector<Node> _nodes;
        std::vector<ServerBlock> _serverBlocks;
        size_t _lines;
        bool _isLoaded;
};

struct Config::Node {
    Config::TokenType   _token;
    std::string _value;
    size_t      _offset;
    size_t      _line;
    size_t      _level;

    Node(TokenType token, std::string value, size_t off, size_t line) : _token(token), _value(value), _offset(off), _line(line) {}
    Node(TokenType token, size_t off, size_t line) : _token(token), _offset(off), _line(line) {}
    Node(TokenType token, size_t off, size_t line, size_t scope_level) : _token(token), _offset(off), _line(line), _level(scope_level) {}
    Node(TokenType token, std::string value, size_t off, size_t line, size_t scope_level) : _token(token), _value(value), _offset(off), _line(line), _level(scope_level) {}
};

class Config;

struct Config::LocationBlock {
    std::vector<std::pair<Config::TokenType, std::string> > _directives;
    std::string _path;
};

class Config;

struct Config::ServerBlock {
    std::vector<LocationBlock> _locations;
    std::vector<std::pair<TokenType, std::string> > _directives;
};
#endif

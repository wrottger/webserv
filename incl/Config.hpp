#ifndef CONFIG_HPP
#define CONFIG_HPP
#include "HttpHeader.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

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
	enum TokenType {
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
		UploadDir
	};

	std::map<std::string, TokenType> _tokens;

	// getters
	static Config &getInstance();
	std::pair<size_t, size_t> getClosestPathMatch(std::string &route, const HttpHeader &header);
	std::pair<size_t, size_t> getClosestPathMatch(const HttpHeader &header);
	std::string getErrorPage(int code, const HttpHeader &header);
	std::string getDirectiveValue(const HttpHeader &header, const Config::TokenType directive);
	std::string getRootDirectory(const HttpHeader &header);
	std::string getFilePath(const HttpHeader &header);
    std::string getFilePath(const HttpHeader &header, std::string Path);
	std::string getDir(const HttpHeader &header);
	Config::LocationBlock& getLocationBlock(std::pair<size_t, size_t> l);
	size_t getMaxBodySize(const HttpHeader &header);
	static std::vector<int> getPorts(std::vector<ServerBlock> &_serverBlocks);
	const std::vector<Node> &getNodes(void) const;
	std::vector<Config::ServerBlock>& getServerBlocks(void);

	// Checkers
	bool isValidPath(const std::string &path);
	bool isDirectiveAllowed(const HttpHeader &header, const Config::TokenType directive, const std::string &value);
	bool isHostSet(const HttpHeader &header);
	bool isCGIAllowed(const HttpHeader &header);
	bool isLoaded(void) const;
	bool isMethodAllowed(const HttpHeader &header, const std::string &method) const;

	// CGI
	std::string getCgiInterpreterPath(const HttpHeader &header);
	std::string getCgiDir(const HttpHeader &header);
	std::string getCgiScriptPath(const HttpHeader &header);

	// config parsing methods
	static bool compareNodes(const Node &a, const Node &b);
	void parseConfigFile(std::string filename);
	void scanTokens(std::ifstream &file);
	void parseScopes(void);
	void buildAST(std::vector<Node>::iterator it, std::vector<Node>::iterator end);
	void parseTokens(void);
	void error(const std::string &msg, const std::vector<Node>::iterator &it);
	void sortVector(std::vector<Node> &vec);
	void addServerBlock(ServerBlock &newBlock, std::vector<Node>::iterator start);
	void printProgressBar(size_t progress, size_t total);
	ServerBlock parseServerBlock(std::vector<Node>::iterator start, std::vector<Node>::iterator end);
	LocationBlock parseLocationBlock(std::vector<Node>::iterator start, std::vector<Node>::iterator end);
	std::vector<TokenInfo> slice(std::string in, std::vector<char> delim);

private:
	Config();
	Config &operator=(const Config &src);
	Config(const Config &src);
	static Config *_instance;

	std::vector<Node> _nodes;
	std::vector<ServerBlock> _serverBlocks;
	size_t _fileSize;
	bool _isLoaded;
};

struct Config::Node {
	Config::TokenType _token;
	std::string _value;
	size_t _offset;
	size_t _line;
	size_t _level;

	Node(TokenType token, std::string value, size_t off, size_t line) :
			_token(token), _value(value), _offset(off), _line(line) {}
	Node(TokenType token, size_t off, size_t line) :
			_token(token), _offset(off), _line(line) {}
	Node(TokenType token, size_t off, size_t line, size_t scope_level) :
			_token(token), _offset(off), _line(line), _level(scope_level) {}
	Node(TokenType token, std::string value, size_t off, size_t line, size_t scope_level) :
			_token(token), _value(value), _offset(off), _line(line), _level(scope_level) {}
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

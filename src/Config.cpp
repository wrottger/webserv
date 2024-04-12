#include "Config.hpp"
#include "Tokens.hpp"
#include "colors.hpp"

Config::Config(void) : _isLoaded(false)
{
    std::cout << GBOLD("Config created") << std::endl;
}

Config::Config(const Config &src)
{
    std::cout << GBOLD("Config copied") << std::endl;
    _tokens = src._tokens;
    _isLoaded = src._isLoaded;
}

Config &Config::operator=(const Config &src)
{
    std::cout << GBOLD("Config assigned") << std::endl;
    if (this != &src)
    {
        _tokens = src._tokens;
        _isLoaded = src._isLoaded;
    }
    return *this;
}

Config::~Config()
{
    std::cout << GBOLD("Config deleted") << std::endl;
}


bool Config::isLoaded(void) const
{
    return _isLoaded;
}

const std::vector<Node>& Config::getNodes(void) const
{
    return _nodes;
}

// This method is supposed to read the config file and tokenize it; output: Nodes with token, value, offset, line, and level;
void Config::scanTokens(std::ifstream &file)
{
    std::vector<char> delim;
    std::string line;
    size_t scopeLevel = 0;
    std::vector<std::pair<std::string, size_t> > out;
    delim.push_back(' ');
    delim.push_back('\t');
    for (size_t n = 0; std::getline(file, line); n++)
    {
        out = slice(line, delim);
        if (out.size() == 0)
            continue;
        for (std::vector<std::pair<std::string, size_t> >::iterator it2 = out.begin(); it2 != out.end(); it2++)
        {
            bool tokenFound = false;
            std::map<std::string, TokenType>::iterator it3 = _tokens.begin();
            for (; it3 != _tokens.end(); it3++)
            {
                if (it3->first == it2->first)
                {
                        if (it3->second == CloseBrace)
                            scopeLevel--;
                        _nodes.push_back(Node(it3->second, it2->second, n, scopeLevel));
                        tokenFound = true;
                        if (it3->second == OpenBrace)
                            scopeLevel++;
                        break;
                }
            }
            if (it2->first[0] == '#')
                _nodes.push_back(Node(Comment, it2->first, it2->second, n, scopeLevel)); 
            else if (tokenFound == false)
                _nodes.push_back(Node(Data, it2->first, it2->second, n, scopeLevel));
        }
    }
    for (std::vector<Node>::iterator it = _nodes.begin(); it != _nodes.end(); it++)
    {
        std::cout << "Token: " << it->_token << " Value: " << it->_value << " Offset: " << it->_offset << " Line: " << it->_line << " Level: " << it->_level << std::endl;
    }
}

// This method is supposed to slice a string into a vector of pairs of strings and their offsets;
std::vector<std::pair<std::string, size_t> > Config::slice(std::string in, std::vector<char> delim)
{
    size_t comment = in.find("#");
    size_t start = 0;
    std::vector<std::pair<std::string, size_t> > out;
    for (size_t i = 0; i < in.size(); i++)
    {
        for (size_t j = 0; j < delim.size(); j++)
        {
            if (in[i] == delim[j] && i < comment)
            {
                if (i > start) {
                    out.push_back(std::make_pair(in.substr(start, i - start), start));
                }
                start = i + 1;
                break;
            }
        }
    }
    if (start < in.size()) {
        out.push_back(std::make_pair(in.substr(start, in.size() - start), start));
    }
    return out;
}

void Config::parseConfigFile(std::string filename)
{
    std::ifstream _fileStream(filename.c_str());
    if (!_fileStream.is_open())
    {
        throw std::runtime_error(RBOLD("Error: Unable to open file " + filename));
    }
    _tokens["server"] = Server;
    _tokens["location"] = Location;
    _tokens["port"] = Port;
    _tokens["host"] = Host;
    _tokens["root"] = Root;
    _tokens["listing"] = Listing;
    _tokens["index"] = Index;
    _tokens["redir"] = Redir;
    _tokens["server_name"] = ServerName;
    _tokens["allowed_methods"] = AllowedMethods;
    _tokens["client_max_body_size"] = ClientMaxBodySize;
    _tokens["default"] = Default;
    _tokens["cgi"] = CGI;
    _tokens["{"] = OpenBrace;
    _tokens["}"] = CloseBrace;

    this->scanTokens(_fileStream);
    _fileStream.close();
    this->parseScopes(_nodes.begin(), _nodes.end());
    this->buildAST(_nodes.begin(), _nodes.end());
    // this->parseTokens();
}

void Config::buildAST(std::vector<Node>::iterator it, std::vector<Node>::iterator end)
{
    for (; it != end; it++)
    if (it->_level == 0)
    {
        if (it->_token != Server && it->_token != Comment && it->_token != OpenBrace && it->_token != CloseBrace)
        {
            std::stringstream ss;
            ss << "Syntax error: unexpected token at row " << it->_line + 1 << ", column " << it->_offset + 1;
            throw std::runtime_error(ss.str());
        }
    }
}

// void Config::parseTokens(void)
// {
//     if(_nodes.size() == 0)
//         throw std::runtime_error("Empty config file");
//     for (std::vector<Node>::iterator it = _nodes.begin(); it != _nodes.end(); it++)
//     {
//         if (it->_level == 0 && it->_token != Server)
//         {
//             std::stringstream ss;
//             ss << "Syntax error: unexpected token '" << _tokens(it->_token) << "' at row " << it->_line + 1 << ", column " << it->_offset + 1;
//             throw std::runtime_error(ss.str());
//         }
//     }
// }

void Config::parseScopes(std::vector<Node>::iterator it, std::vector<Node>::iterator end)
{
    size_t scopeLevel = 0;
    for (; it != end; it++)
    {
        if (it->_token == OpenBrace)
        {
            scopeLevel++;
            _nodes.erase(it);
        }
        else if (it->_token == CloseBrace)
        {
            if (scopeLevel == 0)
            {
                std::stringstream ss;
                ss << "Syntax error: unexpected token at row " << it->_line + 1 << ", column " << it->_offset + 1;
                throw std::runtime_error(ss.str());
            }
            else
            {
                scopeLevel--;
                _nodes.erase(it);
            }
        }

    }
}
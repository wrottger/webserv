#include "Config.hpp"
#include "colors.hpp"
#include "tokens.hpp"

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

void Config::scanTokens(std::ifstream &file)
{
    std::vector<char> delim;
    std::string line;
    std::vector<std::pair<std::string, size_t> > out;
    delim.push_back(' ');
    delim.push_back('\t');
    while (std::getline(file, line))
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
                        _nodes.push_back(Node(it3->second, it2->second));
                        tokenFound = true;
                        break;
                }
            }
            if (it2->first[0] == '#')
                _nodes.push_back(Node(Comment, it2->first, it2->second)); 
            else if (tokenFound == false)
                _nodes.push_back(Node(Data, it2->first, it2->second));
        }
    }
}

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
}

// void Config::checkScopes(void)
// {
//     size_t scopeLevel = 0;
//     std::vector<std::pair<size_t, size_t> > scopes; // pair of start and end of scope
//     for (std::vector<std::string>::iterator it = _fileContent.begin(); it != _fileContent.end(); it++)
//     {
//         if (it->find("{") != std::string::npos)
//         {
//             scopes.push_back(std::make_pair(it - _fileContent.begin(), 0));
//             scopeLevel++;
//         }
//         else if (it->find("}") != std::string::npos)
//         {
//             if (scopeLevel == 0)
//                 std::cout << RBOLD("Syntax error: unexpected '}' at row ") << it - _fileContent.begin() + 1 << RBOLD(", column ") << it->find("}") + 1 << std::endl;
//             else
//             {
//                 scopes[scopeLevel - 1].second = it - _fileContent.begin();
//                 scopeLevel--;
//             }
//         }
//     }
//     for (std::vector<std::pair<size_t, size_t> >::iterator it = scopes.begin(); it != scopes.end(); it++)
//     {
//         if (it->second == 0)
//             std::cout << RBOLD("Syntax error: missing '}' at row ") << it->first + 1 << RBOLD(", column ") << _fileContent[it->first].find("{") + 1 << std::endl;
//     }
// }
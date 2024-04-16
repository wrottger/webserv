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
    _tokens["index"] = Index;
    _tokens["redir"] = Redir;
    _tokens["server_name"] = ServerName;
    _tokens["allowed_methods"] = AllowedMethods;
    _tokens["client_max_body_size"] = ClientMaxBodySize;
    _tokens["default"] = Default;
    _tokens["cgi"] = CGI;
    _tokens["{"] = OpenBrace;
    _tokens["}"] = CloseBrace;
    _tokens[";"] = Semicolon;

    this->scanTokens(_fileStream);
    _fileStream.close();
    this->parseScopes();
    this->buildAST(_nodes.begin(), _nodes.end());
    // for (std::vector<ServerBlock>::iterator it = _serverBlocks.begin(); it != _serverBlocks.end(); it++)
    // {
    //     std::cout << "Server block:" << std::endl;
    //     for (std::map<std::string, std::string>::iterator it2 = it->_directives.begin(); it2 != it->_directives.end(); it2++)
    //     {
    //         std::cout << it2->first << ": " << it2->second << std::endl;
    //     }
    //     for (std::vector<LocationBlock>::iterator it2 = it->_locations.begin(); it2 != it->_locations.end(); it2++)
    //     {
    //         std::cout << "Location block:" << std::endl;
    //         for (std::map<std::string, std::string>::iterator it3 = it2->_directives.begin(); it3 != it2->_directives.end(); it3++)
    //         {
    //             std::cout << it3->first << ": " << it3->second << std::endl;
    //         }
    //     }
    // }
}

// This method is supposed to read the config file and tokenize it; output: Nodes with token, value, offset, line, and level;
void Config::scanTokens(std::ifstream &file)
{
    std::vector<char> delim;
    std::string line;
    std::vector<std::pair<std::string, size_t> > out;
    delim.push_back(' ');
    delim.push_back('\t');
    delim.push_back('{');
    delim.push_back('}');
    delim.push_back(';');
    for (size_t n = 0; std::getline(file, line); n++)
    {
        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == '{')
                _nodes.push_back(Node(OpenBrace, i, n));
            else if (line[i] == '}')
                _nodes.push_back(Node(CloseBrace, i, n));
            else if (line[i] == ';')
                _nodes.push_back(Node(Semicolon, i, n));
        }
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
                        _nodes.push_back(Node(it3->second, it2->second, n));
                        tokenFound = true;
                        break;
                }
            }
            if (it2->first[0] == '#')
                continue;
            else if (tokenFound == false)
            {
                _nodes.push_back(Node(Data, it2->first, it2->second, n));
            }
        }
    }
    this->sortVector(_nodes);
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

// This method is supposed to parse the scopes of the config file, check for syntax errors and delete braces and comments;
void Config::parseScopes(void)
{
    std::stack<Node> openBraces;
    size_t level = 0;
    for (std::vector<Node>::iterator it = _nodes.begin(); it != _nodes.end(); it++)
    {
        if (it->_token == OpenBrace)
        {
            openBraces.push(*it);
            level++;
        }
        it->_level = level;
        if (it->_token == CloseBrace)
        {
            if (level)
            {
                openBraces.pop();
                level--;
            }
            else
               error("Syntax error: unexpected brace", it);
        }
    }
    if (!openBraces.empty())
    {
        Node unclosedBrace = openBraces.top();
        std::stringstream ss;
        ss << "Syntax error: unclosed brace at row " << unclosedBrace._line + 1 << ", column " << unclosedBrace._offset + 1;
        throw std::runtime_error(RBOLD(ss.str()));
    }
    for (std::vector<Node>::reverse_iterator it = _nodes.rbegin(); it != _nodes.rend(); it++)
    {
        if (it->_token == OpenBrace || it->_token == CloseBrace)
            _nodes.erase(it.base() - 1);
    }
}

void Config::error(const std::string &msg, const std::vector<Node>::iterator& it)
{
	std::stringstream ss;
	ss << msg << " at row " << it->_line + 1 << ", column " << it->_offset + 1;
	throw std::runtime_error(RBOLD(ss.str()));
}

// Sorts the vector of nodes by order of line number and offset in the config file;
void Config::sortVector(std::vector<Node>& vec)
{
    for (size_t i = 0; i < vec.size(); i++)
    {
        for (size_t j = i + 1; j < vec.size(); j++)
        {
            if (vec[j]._line < vec[i]._line || (vec[j]._line == vec[i]._line && vec[j]._offset < vec[i]._offset))
            {
                Node temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            }
        }
    }
}

void Config::buildAST(std::vector<Node>::iterator it, std::vector<Node>::iterator end)
{

    for (; it != end; it++)
    {
        if (!it->_level)
        {
            if (it->_token == Server)
            {
                it++;
                if (it == end)
                    error("Syntax error: unexpected end of file", it);
                std::vector<Node>::iterator start = it;
                while (it->_level)
                    it++;
                ServerBlock serverBlock = parseServerBlock(start, it);
                _serverBlocks.push_back(serverBlock);
            }
            else
                error("Syntax error: token outside of server block", it);
        }
        else
            continue;
    }
}


ServerBlock Config::parseServerBlock(std::vector<Node>::iterator& it, std::vector<Node>::iterator& end)
{
    ServerBlock block;
    for (; it != end; it++)
    {
        if (it->_token == Location)
        {
            std::cout << "Location block" << std::endl;
            std::vector<Node>::iterator locationStart = it;
            while (it->_level > locationStart->_level)
                it++;
            LocationBlock locationBlock = parseLocationBlock(locationStart, it);
            block._locations.push_back(locationBlock);
        }
        else
        {
            std::cout << "Server directives" << std::endl;
            switch (it->_token)
            {
                case Port:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives[Port] = (it + 1)->_value;
                        it++;
                    }
                    else
                        error("Syntax error: port directive requires a value", it);
                    break;
                case Host:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives[Host] = (it + 1)->_value;
                        it++;
                    }
                    else
                        error("Syntax error: host directive requires a value", it);
                    break;
                case ServerName:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives[ServerName] = (it + 1)->_value;
                        it++;
                    }
                    else
                        error("Syntax error: server_name directive requires a value", it);
                    break;
                case ClientMaxBodySize:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives[ClientMaxBodySize] = (it + 1)->_value;
                        it++;
                    }
                    else
                        error("Syntax error: client_max_body_size directive requires a value", it);
                    break;
                case Semicolon:
                    break;
                default:
                    error("Syntax error: unexpected token in server block", it);
            }
        }
    }
    for (std::map<TokenType, std::string>::iterator it2 = block._directives.begin(); it2 != block._directives.end(); it2++)
    {
        std::cout << it2->first << ": " << it2->second << std::endl;
    }
    return block;
}

LocationBlock Config::parseLocationBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end)
{
    LocationBlock block;
    for (std::vector<Node>::iterator it = start; it != end; it++)
    {
        std::cout << "Location directives" << std::endl;
        if (it->_token != Data)
            error("Syntax error: expected path in location block", it);
        else 
            block._path = it->_value;
        switch (it->_token)
        {
            case Root:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[Root] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: root directive requires a value", it);
                break;
            case Index:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[Index] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: index directive requires a value", it);
                break;
            case Redir:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[Redir] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: redir directive requires a value", it);
                break;
            case AllowedMethods:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[AllowedMethods] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: allowed_methods directive requires a value", it);
                break;
            case Default:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[Default] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: default directive requires a value", it);
                break;
            case CGI:
                if (it + 1 != end && (it + 1)->_token == Data)
                {
                    block._directives[CGI] = (it + 1)->_value;
                    it++;
                }
                else
                    error("Syntax error: cgi directive requires a value", it);
                break;
            case Semicolon:
                break;
            default:
                error("Syntax error: unexpected token in location block", it);
        }
    }
    for (std::map<TokenType, std::string>::iterator it = block._directives.begin(); it != block._directives.end(); it++)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    return block;
}

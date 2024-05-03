#include "Config.hpp"
#include "colors.hpp"


Config* Config::_instance = NULL;

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
    _tokens["root"] = Root;
    _tokens["index"] = Index;
    _tokens["listing"] = Listing;
    _tokens["redir"] = Redir;
    _tokens["server_name"] = ServerName;
    _tokens["allowed_methods"] = AllowedMethods;
    _tokens["client_max_body_size"] = ClientMaxBodySize;
    _tokens["default"] = Default;
    _tokens["cgi"] = CGI;
    _tokens["{"] = OpenBrace;
    _tokens["}"] = CloseBrace;
    _tokens[";"] = Semicolon;
    _tokens["error"] = Error;
    _tokens["error_page"] = ErrorPage;

    this->scanTokens(_fileStream);
    _fileStream.close();
    this->parseScopes();
    this->buildAST(_nodes.begin(), _nodes.end());
    if (_serverBlocks.empty())
        throw std::runtime_error(RBOLD("Error: no server blocks found in config file"));
    _isLoaded = true;
}

//iterate through the server blocks and check if the server_name and ports are unique across all server blocks
void Config::addServerBlock(ServerBlock& newBlock, std::vector<Node>::iterator& start)
{
    static std::map<std::pair<std::string, std::string>, bool> hostPortMap;
    std::string serverName;
    std::vector<std::string> ports;

    for (std::vector<std::pair<TokenType, std::string> >::iterator it = newBlock._directives.begin(); it != newBlock._directives.end(); it++) {
        switch (it->first) {
            case ServerName:
                serverName = it->second;
                break;
            case Port:
                ports.push_back(it->second);
                break;
            default:
                break;
        }
    }
    for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); it++) {
        std::pair<std::string, std::string> hostPortPair(serverName, *it);
        if (hostPortMap.find(hostPortPair) != hostPortMap.end()) {
            error("Duplicate server name and port combination", start);
        }
        hostPortMap[hostPortPair] = true;
    }
    _serverBlocks.push_back(newBlock);
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
        //adds delimiters to the vector of nodes
        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == '{')
                _nodes.push_back(Node(OpenBrace, i, n));
            else if (line[i] == '}')
                _nodes.push_back(Node(CloseBrace, i, n));
            else if (line[i] == ';')
                _nodes.push_back(Node(Semicolon, i, n));
        }
        out = slice(line, delim); //slices the line into pure data tokens i.e. all tokens except delimiters
        if (out.size() == 0) //skips empty lines
            continue;
        for (std::vector<std::pair<std::string, size_t> >::iterator it2 = out.begin(); it2 != out.end(); it2++)
        {
            bool tokenFound = false;
            std::map<std::string, TokenType>::iterator it3 = _tokens.begin();
            //checks if the token is a valid token
            for (; it3 != _tokens.end(); it3++)
            {
                if (it3->first == it2->first)
                {
                        _nodes.push_back(Node(it3->second, it2->second, n));
                        tokenFound = true;
                        break;
                }
            }
            if (it2->first[0] == '#') //skips comments
                continue;
            else if (tokenFound == false) //all other tokens are considered data
            {
                _nodes.push_back(Node(Data, it2->first, it2->second, n));
            }
        }
        _lines = n;
    }
    this->sortVector(_nodes); // sorts the vector of nodes by order of line number and offset in the config file
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
    //erase braces
    for (std::vector<Node>::iterator it = _nodes.begin(); it != _nodes.end();)
    {
        if (it->_token == OpenBrace || it->_token == CloseBrace)
            it = _nodes.erase(it);
        else
            ++it;
    }
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
    while (it != end)
    {
        if (!it->_level)
        {
            if (it->_token == Server)
            {
                std::vector<Node>::iterator start = it; // starting node of the server block
                std::vector<Node>::iterator start2 = start;
                it++;
                if (it == end)
                    error("Syntax error: incomplete server block", start);
                while (it->_level && it != end) // iterate until the end of the server block
                    it++;
                if (start + 1 == it)
                    error("Syntax error: incomplete server block", start);
                start++;
                // parse the server block and add it to the vector of server blocks
                ServerBlock serverBlock = parseServerBlock(start, it);
                if (serverBlock._directives.size())
                {
                    addServerBlock(serverBlock , start2);
                    if (it->_line < _lines)
                        printProgressBar(it->_line, _lines);
                }
                else
                    error("Syntax error: incomplete server block", start);
                continue;
            }
            else
                error("Syntax error: token not within server block", it);
        }
        else
            error("Syntax error: token not within server block", it);
    }
}

// This method is supposed to parse the server block and its directives;
Config::ServerBlock Config::parseServerBlock(std::vector<Node>::iterator& it, std::vector<Node>::iterator& end)
{
    ServerBlock block;
    std::vector<Node>::iterator start = it - 1;
    while (it != end)
    {
        // create a new location block and add it to the server block
        if (it->_token == Location && it->_level == 1)
        {
            it++;
            if (it == end)
                error("Syntax error: expected location block", it);
            std::vector<Node>::iterator locationStart = it; // starting node of the location block
            it++;
            if (it == end)
                error("Syntax error: expected location block", it);
            while (it != end && it->_level > locationStart->_level) // iterate until the end of the location block
                it++;
            // parse the location block and add it to the vector of location blocks
            LocationBlock locationBlock = parseLocationBlock(locationStart, it);
            if (!locationBlock._path.empty() && locationBlock._directives.size())
                block._locations.push_back(locationBlock);
            else
                error ("Syntax error: incomplete location block", locationStart - 1);
        }
        // parse directives in server block
        else
        {
            switch (it->_token)
            {
                case Root:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        // make pair of token and value and add it to vector of directives
                        block._directives.push_back(std::make_pair(Root, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after root directive", it - 2);
                    }
                    else
                        error("Syntax error: root directive requires a value", it);
                    break;
                case Port:
                    // check if the value is a number and within the range of valid port numbers
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(Port, (it + 1)->_value));
                        char *endptr; // used to check if strtol was successful
                        long portNumber = strtol((it + 1)->_value.c_str(), &endptr, 10);
                        if (errno == ERANGE || *endptr != '\0')
                            error("Syntax error: invalid port number", it + 1);
                        else if (portNumber < 0 || portNumber > 65535)
                            error("Syntax error: port number out of range", it + 1);
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after port directive", it - 2);
                    }
                    else
                        error("Syntax error: port directive requires a value", it);
                    break;
                case CGI:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(CGI, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after cgi directive", it - 2);
                    }
                    else
                        error("Syntax error: cgi directive requires a value", it);
                    break;
                case ServerName:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(ServerName, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after server_name directive", it - 2);
                    }
                    else
                        error("Syntax error: server_name directive requires a value", it);
                    break;
                case ClientMaxBodySize:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(ClientMaxBodySize, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after client_max_body_size directive", it - 2);
                    }
                    else
                        error("Syntax error: client_max_body_size directive requires a value", it);
                    break;
                case Semicolon:
                {
                    it++;
                    break;
                }
                case ErrorPage:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        std::pair <TokenType, std::string> pair;
                        pair = std::make_pair(ErrorPage, (it + 1)->_value);
                        it += 2;
                        if (it->_token != Data)
                            error("Syntax error: missing path to error page", it - 2);
                        else if (isValidPath((it)->_value))
                            pair.second += " " + (it)->_value;
                        else
                            error("Syntax error: invalid path in error page directive", it);
                        it++;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after error page directive", it - 2);
                        block._directives.push_back(pair);
                    }
                    else
                        error("Syntax error: error page directive requires error code + error page path", it);
                    break;
                default:
                    error("Syntax error: unexpected token in server block", it);
            }
        }
    }
    // check if there are any missing directives
    bool portFound = false;
    size_t root = 0;
    size_t serverName = 0;
    for (std::vector<std::pair<TokenType, std::string> >::iterator it2 = block._directives.begin(); it2 != block._directives.end(); it2++)
    {
        switch (it2->first)
        {
            case Port:
            {
                portFound = true;
                continue;
            }
            case ServerName:
            {
                serverName++;
                continue;
            }
            case Root:
            {
                root++;
                continue;
            }
            default:
                continue;
        }
    }
    if (!root && block._locations.empty())
        error("Syntax error: server requests cannot be handled without a port number or location block", start);
    else if (root > 1)
        error("Syntax error: server block can only have one root directive", start);
    else if (!portFound)
        error("Syntax error: server block requires a port directive", start);
    else if (!serverName)
        error("Syntax error: server block requires a server_name directive", start);
    else if (serverName > 1)
        error("Syntax error: server block can only have one server_name directive", start);
    return block;
}

Config::LocationBlock Config::parseLocationBlock(std::vector<Node>::iterator& start, std::vector<Node>::iterator& end)
{
    LocationBlock block;
    std::vector<Node>::iterator it = start;
    std::string methods;

    while (it != end)
    {
        if (it == start)
        {
            if (it->_token != Data)
                error("Syntax error: expected path in location block", it);
            else if (isValidPath(it->_value))
            {
                block._path = it->_value;
                it++;
            }
            else
                error("Syntax error: invalid path in location block", it);
        }
        else
        {
            switch (it->_token)
            {
                case Listing:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(Listing, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after listing directive", it - 2);
                    }
                    else
                        error("Syntax error: listing directive requires a value", it);
                    break;
                case Root:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        if (isValidPath((it + 1)->_value))
                            block._directives.push_back(std::make_pair(Root, (it + 1)->_value));
                        else
                            error("Syntax error: invalid path in root directive", it + 1);
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after root directive", it - 2);
                    }
                    else
                        error("Syntax error: root directive requires a value", it);
                    break;
                case ErrorPage:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        std::pair <TokenType, std::string> pair;
                        pair = std::make_pair(ErrorPage, (it + 1)->_value);
                        it += 2;
                        if (it->_token != Data)
                            error("Syntax error: missing path to error page", it - 2);
                        else if (isValidPath((it)->_value))
                            pair.second += " " + (it)->_value;
                        else
                            error("Syntax error: invalid path in error page directive", it);
                        it++;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after error page directive", it - 2);
                        block._directives.push_back(pair);
                    }
                    else
                        error("Syntax error: error page directive requires error code + error page path", it);
                    break;
                case Index:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(Index, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after index directive", it - 2);
                    }
                    else
                        error("Syntax error: index directive requires a value", it);
                    break;
                case Redir:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(Redir, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after redir directive", it - 2);
                    }
                    else
                        error("Syntax error: redir directive requires a value", it);
                    break;
                case AllowedMethods:
                    it++;
                    if (it->_token != Data)
                        error("Syntax error: expected method in allowed_methods directive", it);
                    while (it != end && it->_token != Semicolon)
                    {
                        if (it->_token == Data)
                        {
                            if (it->_value == "GET" || it->_value == "POST" || it->_value == "DELETE")
                            {
                                if (!methods.empty())
                                    methods += " ";
                                methods += it->_value;
                                it++;
                            }
                            else
                                error("Syntax error: invalid method in allowed_methods directive", it);
                        }
                        else
                            error("Syntax error: token not allowed in allowed_methods directive", it);
                    }
                    if (it == end)
                        error("Syntax error: missing semicolon after allowed_methods directive", it);
                    block._directives.push_back(std::make_pair(AllowedMethods, methods));
                    break;
                case Default:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        block._directives.push_back(std::make_pair(Default, (it + 1)->_value));
                        it += 2;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after default directive", it - 2);
                    }
                    else
                        error("Syntax error: default directive requires a value", it);
                    break;
                case CGI:
                    if (it + 1 != end && (it + 1)->_token == Data)
                    {
                        std::pair <TokenType, std::string> pair;
                        pair = std::make_pair(CGI, (it + 1)->_value);
                        it += 2;
                        if (it->_token != Data)
                            error("Syntax error: missing interpreter path in cgi directive", it - 2);
                        it++;
                        if (it == end || it->_token != Semicolon)
                            error("Syntax error: missing semicolon after cgi directive", it - 2);
                        pair.second += " " + (it - 1)->_value;
                        block._directives.push_back(pair);
                    }
                    else
                        error("Syntax error: cgi directive requires file extension + interpreter path", it);
                    break;
                case Semicolon:
                {
                    it++;
                    break;
                }
                default:
                    error("Syntax error: unexpected token in location block", it);
            }
        }
    }
    return block;
}

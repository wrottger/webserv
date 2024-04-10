#ifndef TOKENS_HPP
#define TOKENS_HPP

# include <string>
# include <vector>
enum TokenType {
    OpenBrace,
    CloseBrace,
    Semicolon,
    Server,
    Location,
    Port,
    Host,
    Root,
    Listing,
    Index,
    Redir,
    ServerName,
    AllowedMethods,
    ClientMaxBodySize,
    Default,
    CGI,
};

struct Node {
    TokenType   _token;
    std::string _value;
    size_t      _offset;
    size_t      _scopeLevel;
    std::vector<Node> _children;

    Node(TokenType token, std::string value, size_t off) : _token(token), _value(value), _offset(off) {}
};

#endif
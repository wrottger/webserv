#ifndef TOKENS_HPP
#define TOKENS_HPP

# include <string>
# include <vector>
enum TokenType {
    OpenBrace,
    CloseBrace,
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
    Comment,
    Data,
};

struct Node {
    TokenType   _token;
    std::string _value;
    size_t      _offset;
    size_t      _line;

    Node(TokenType token, size_t off, size_t line) : _token(token), _offset(off), _line(line) {}
    Node(TokenType token, std::string value, size_t off, size_t line) : _token(token), _value(value), _offset(off), _line(line) {}
};

#endif
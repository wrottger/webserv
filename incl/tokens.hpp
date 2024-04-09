#ifndef TOKENS_HPP
#define TOKENS_HPP

# include <string>

std::string tokenTypes[] = {
"server",
"location",
};

struct Token {
    std::string _type;
    std::string _value;
    size_t      _offset;

    Token(std::string type, size_t off) : _type(type), _offset(off) {}
};

#endif
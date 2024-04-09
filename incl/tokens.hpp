#ifndef TOKENS_HPP
#define TOKENS_HPP

# include <string>

std::string tokenTypes[] = {
"Server",
"Location",
};

struct Token {
    std::string type;
    std::string value;
    size_t      offset;
};

#endif
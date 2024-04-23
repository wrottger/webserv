#ifndef LOCATION_BLOCK_HPP
# define LOCATION_BLOCK_HPP
# include <string>
# include <vector>
# include "Tokens.hpp"

struct LocationBlock {

    std::vector<std::pair<TokenType, std::string> > _directives;
    std::string _path;

};

#endif
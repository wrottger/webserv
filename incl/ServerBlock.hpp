#ifndef SERVER_BLOCK_HPP
# define SERVER_BLOCK_HPP
# include <vector>
# include "LocationBlock.hpp"

struct ServerBlock {

    std::vector<LocationBlock> _locations;
    std::vector<std::pair<TokenType, std::string> > _directives;

};

#endif
#ifndef LOCATION_BLOCK_HPP
# define LOCATION_BLOCK_HPP
# include <map>
# include <string>

struct LocationBlock {

    std::map <std::string, std::string> _directives;
    std::string _path;

};

#endif
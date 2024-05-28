#ifndef UTILS_HPP
# define UTILS_HPP
# include <string>
# include <sys/stat.h>
# include <sstream>
# include <limits>
#include <algorithm>

namespace Utils {
	bool isFolder(const std::string &path);
	bool isFile(const std::string &path);
	std::string toString(size_t number);
	std::string toString(int number);
	int stringToNumber(const std::string& s);
	char toLower(char c);
	std::string toLowerString(const std::string& str);

	template <typename T>
	T stringToNumber(const std::string& s) {
    	std::stringstream ss(s);
    	T num = 0;
    	ss >> num;
		if (ss.fail() || !ss.eof())
		{
			throw std::invalid_argument("Invalid number");
		}
		if (!std::numeric_limits<T>::is_signed) {
            // Check if the string represents a negative number
            if (s.find('-') != std::string::npos) {
                throw std::invalid_argument("Invalid number: negative value for unsigned type");
            }
        }
    	return num;
	}
}

#endif
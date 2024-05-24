#ifndef UTILS_HPP
#define UTILS_HPP
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string>

namespace Utils {
bool isFolder(const std::string &path);
bool isFile(const std::string &path);
std::string toString(size_t number);
std::string toString(int number);
int stringToNumber(const std::string &s);
char toLower(char c);
std::string toLowerString(const std::string &str);
} //namespace Utils

#endif
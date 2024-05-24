#include "Utils.hpp"

bool Utils::isFolder(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool Utils::isFile(const std::string &path)
{
	struct	stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

std::string Utils::toString(size_t number) {
	std::stringstream result;
	result << number;
	return result.str();
}

std::string Utils::toString(int number) {
	std::stringstream result;
	result << number;
	return result.str();
}

// If the string is not a number, it will return 0
int Utils::stringToNumber(const std::string& s) {
	std::stringstream ss(s);
	int num;
	ss >> num;
	return num;
}

char Utils::toLower(char c) {
	return std::tolower(static_cast<unsigned char>(c));
}

std::string Utils::toLowerString(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), Utils::toLower);
	return result;
}

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

#include "Utilities.hpp"

bool Utilities::isFolder(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool Utilities::isFile(const std::string &path)
{
	struct	stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

std::string Utilities::toString(size_t number) {
	std::stringstream result;
	result << number;
	return result.str();
}

std::string Utilities::toString(int number) {
	std::stringstream result;
	result << number;
	return result.str();
}

// If the string is not a number, it will return 0
int Utilities::stringToNumber(const std::string& s) {
	std::stringstream ss(s);
	int num;
	ss >> num;
	return num;
}

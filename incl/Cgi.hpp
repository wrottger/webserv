#ifndef CGI_HPP
# define CGI_HPP
#include <ctime>
#include <iostream>
#include "Config.hpp"
#include "HttpHeader.hpp"

class Cgi {
	private:
		std::string _outputBuffer;
		int _sockets[2];
		time_t _lastModified;
		bool _isFinished;
		int _errorCode;

	private:
		Cgi();
		Cgi(const Cgi &other);
		Cgi &operator =(const Cgi &other);

		
		const char* createEnviroment();
	public:

		Cgi(std::string &bodyBuffer, int epollFd, HttpHeader &requestObject);
		~Cgi();

		bool isFinished();
		int getErrorCode();
};

#endif
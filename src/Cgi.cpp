#include "Cgi.hpp"

const char *Cgi::createEnviroment() {
	return NULL;
}

Cgi::Cgi(std::string &bodyBuffer, int epollFd, HttpHeader &requestObject) {
	(void)bodyBuffer;
	(void)epollFd;
	(void)requestObject;
}

bool Cgi::isFinished() {
	return _isFinished;
}

int Cgi::getErrorCode() {
	return _errorCode;
}

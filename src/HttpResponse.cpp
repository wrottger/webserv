#include "HttpResponse.hpp"

HttpResponse::HttpResponse(HttpHeader &header, int fds) {
}

size_t HttpResponse::readBuffer(const char *buffer) {
	return size_t();
}

void HttpResponse::canWrite(bool canWrite) {
}

bool HttpResponse::finished() {
	return false;
}

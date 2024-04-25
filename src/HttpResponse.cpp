#include "HttpResponse.hpp"

HttpResponse::HttpResponse(HttpHeader &header, int fds) {
}

size_t HttpResponse::readBuffer(const char *buffer) {
	return size_t();
}

void HttpResponse::write() {
}

bool HttpResponse::finished() {
	return false;
}

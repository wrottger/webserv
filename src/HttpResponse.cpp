#include "HttpResponse.hpp"
#include <sys/socket.h>

HttpResponse::HttpResponse(HttpHeader &header, int fds) : header(header), fds(fds) {
	response = "HTTP/1.1 ";
	if (header.isError())
	{
		response += header.getError().code() + " ";
		response += header.getError().message() + "\r\n";
	}
}

void HttpResponse::write() {
	if (header.isError()) {
		send(fds, response.c_str(), response.size(), 0);
	} else if (isChunked) {
		// write
	} else {
	}
}

bool HttpResponse::finished() {
	return isFinished;
}

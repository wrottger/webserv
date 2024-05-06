#include "HttpResponse.hpp"
#include <sys/socket.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "Config.hpp"

std::string HttpResponse::generateErrorResponse(int code, const std::string &message) {
	std::string error_path = config->getErrorPage(error.code(), header.getPath(), header.getHeader("host"));
	response = "HTTP/1.1 ";
	if (error_path.empty())
	{
		char *code = new char[4];
		sprintf(code, "%d", error.code());
		std::string error_html = "<HTML><body><p><strong>";
		error_html += code;
		error_html += " </strong>";
		error_html += message;
		error_html += "</p></body>";

		response += code;
		response += " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: ";
		char *length = new char[4];
		sprintf(length, "%ld", error_html.size());
		response += length;
		response += "\r\n\r\n";
		response += error_html;
	}
	else
	{
		std::ifstream file(config->getFilePath(error_path, header.getHeader("host")).c_str());
		if (!file.is_open())
		{
			response += "HTTP/1.1 500 Internal Server Error\r\n";
			response += "Connection: close\r\n\r\n";
			return response;
		}
		response += code + " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n\r\n";
		response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	LOG_DEBUG_WITH_TAG(response, "HttpResponse::generateErrorResponse");
	return response;
}

HttpResponse::HttpResponse(HttpHeader &header, int fds) : header(header), fds(fds) {
	LOG_DEBUG("HttpResponse::HttpResponse");
	config = Config::getInstance();
	response = "HTTP/1.1 ";
	isFinished = false;
	error = header.getError();
	if (error.code() != 0)
	{
		response = generateErrorResponse(error.code(), error.message());
		return;
	}

	if (header.getMethod() == "GET")
	{
		if (config->isDirectiveAllowed(header.getPath(), header.getHeader("host"), Config::AllowedMethods, "GET"))
		{
			std::string filePath = config->getFilePath(header.getPath(), header.getHeader("host"));
			getFile.open(filePath.c_str());
			if (!getFile.is_open())
				error = HttpError(404, "Not Found");
			response += "transfer-encoding: chunked\r\n\r\n";
		}
		else
			error = HttpError(405, "Method Not Allowed");
	}
	else if (header.getMethod() == "POST")
	{
		error = HttpError(405, "Method Not Allowed");
	}
	else if (header.getMethod() == "DELETE")
	{
		if (!config->isDirectiveAllowed(header.getPath(), header.getHeader("host"), Config::AllowedMethods, "DELETE"))
			error = HttpError(405, "Method Not Allowed");
		else if (remove(header.getPath().c_str()) != 0)
			error = HttpError(500, "Internal Server Error");
		else {
			response += "200 OK\r\n";
			response += "<html><body><h1>File deleted</h1></body></html>\r\n";
		}
	}
	else
	{
		error = HttpError(405, "Method Not Allowed");
	}
	if (error.code() != 0)
		response = generateErrorResponse(error.code(), error.message());
}

HttpResponse::~HttpResponse() {
}

size_t HttpResponse::readBuffer(const char *buffer) {
	(void)buffer;
	if (error.code() != 0)
		return 0;
	return 0;
}

void HttpResponse::write() {
	if (isFinished)
	{
		LOG_INFO("HttpResponse::write isFinished");
		return;
	}

	if (error.code() != 0 || header.getMethod() == "DELETE"){
		size_t sentBytes =  send(fds, response.c_str(), response.size(), 0);
		if (sentBytes != response.size())
			response = response.substr(sentBytes);
		if (response.empty())
			isFinished = true;
	} else if (header.getMethod() == "GET") {
		LOG_DEBUG("HttpResponse::sending GET response");
		if (!response.empty())
		{
			size_t sentBytes =  (size_t) send(fds, response.c_str(), response.size(), 0);
			if (sentBytes != response.size())
				response = response.substr(sentBytes);
		} else if (getFile.eof()) {
			response = "0\r\n\r\n";
			size_t sentBytes =  (size_t) send(fds, response.c_str(), response.size(), 0);
			if (sentBytes != response.size())
				response = response.substr(sentBytes);
			if (response.empty())
				isFinished = true;
		} else {
			getFile.read(ChunkedBuffer, 1024);
			size_t readBytes = getFile.gcount();
			response += readBytes;
			response += "\r\n";
			response += ChunkedBuffer;
			response += "\r\n";
		}

	} else {
		LOG_ERROR("HttpResponse::IMPOSSIBLE STATE");
	}
}

bool HttpResponse::finished() {
	return isFinished;
}

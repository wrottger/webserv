#include "HttpResponse.hpp"
#include <sys/socket.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse::HttpResponse(HttpHeader &header, int fds) : header(header), fds(fds) {
	config = Config::getInstance();
	response = "HTTP/1.1 ";
	error = header.getError();
	if (error.code() != 0)
	{
		response += header.getError().code() + " ";
		response += header.getError().message() + "\r\n";
		response += "Connection: close\r\n\r\n";
		return;
	}

	if (header.getMethod() == "GET")
	{
		if (config->isDirectiveAllowed(header.getPath(), header.getHeader("host"), Config::AllowedMethods, "GET"))
			response += "transfer-encoding: chunked\r\n\r\n";
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
	{
		config->getDirectiveValue(header.getPath(), header.getHeader("host"), Config::ErrorPage);
		response += header.getError().code() + " ";
		response += header.getError().message() + "\r\n";
		response += "Connection: close\r\n\r\n";
	}

	if (header.getMethod() == "GET")
	{
		if (config->isDirectiveAllowed(header.getPath(), header.getHeader("host"), Config::AllowedMethods, "GET"))
		{
			LOG_DEBUG("GET ALLOWED");
			std::string filePath = config->getFilePath(header.getPath(), header.getHeader("host"));
			getFile.open(filePath.c_str());
			if (!getFile.is_open())
				error = HttpError(404, "Not Found");
			response += "200 OK\r\n";
			response += "Connection: close\r\n";
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
			response += "200 OK\r\n\r\n";
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
		return;

	if (error.code() != 0) {
		size_t sentBytes =  (size_t) send(fds, response.c_str(), response.size(), 0);
		if (sentBytes != response.size())
			response = response.substr(sentBytes);
		if (response.empty())
			isFinished = true;
	} else {
		
	}
}

bool HttpResponse::finished() {
	return isFinished;
}

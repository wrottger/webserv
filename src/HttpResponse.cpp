#include "HttpResponse.hpp"
#include <sys/socket.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "Config.hpp"

std::string HttpResponse::generateErrorResponse(const std::string &message) {
	std::string error_path = config->getErrorPage(error.code(), header.getPath(), header.getHeader("host"));
	response = "HTTP/1.1 ";
	std::stringstream errCode;
	errCode << error.code();
	if (error_path.empty())
	{
		std::string error_html = "<HTML><body><p><strong>";
		error_html += errCode.str();
		error_html += " </strong>";
		error_html += message;
		error_html += "</p></body>";

		response += errCode.str();
		response += " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: ";
		std::stringstream errSize;
		errSize << error_html.size();
		response += errSize.str();
		response += "\r\n\r\n";
		response += error_html;
	}
	else
	{
		std::ifstream file(config->getFilePath(error_path, header.getHeader("host")).c_str());
		if (!file.is_open())
		{
			error = HttpError(500, "Couldn't open error file");
			response = generateErrorResponse(error.message());
			return response;
		}
		response += errCode.str() + " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n\r\n";
		response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	LOG_DEBUG_WITH_TAG(response, "HttpResponse::generateErrorResponse");
	return response;
}

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

HttpResponse::HttpResponse(HttpHeader &header, int fds) : header(header), fds(fds) {
	LOG_DEBUG("HttpResponse::HttpResponse");
	config = Config::getInstance();
	response = "HTTP/1.1 ";
	isFinished = false;
	error = header.getError();
	if (error.code() != 0)
	{
		response = generateErrorResponse(error.message());
		return;
	}

	if (header.getMethod() == "GET")
	{
		LOG_DEBUG("GET");
		if (ends_with(header.getPath(), "/") && config->getDirectiveValue(header.getPath(), header.getHost(), Config::Index).size() > 0)
		{
			LOG_DEBUG("GET INDEX");
			std::string filePath = header.getPath() + "/index.html";
			getFile.open(filePath.c_str());
			if (getFile.fail())
			{
				LOG_DEBUG("Couldn't open file");
				error = HttpError(404, "Not Found");
				response = generateErrorResponse(error.message());
				return;
			}
			response += "200 OK\r\n";
			response += "Connection: close\r\n";
			response += "transfer-encoding: chunked\r\n\r\n";
		}
		else if (config->isDirectiveAllowed(header.getPath(), header.getHeader("host"), Config::AllowedMethods, "GET"))
		{
			LOG_DEBUG("GET FILE");
			std::string filePath = config->getFilePath(header.getPath(), header.getHeader("host"));
			LOG_DEBUG(filePath);
			getFile.open(filePath.c_str());
			if (getFile.fail())
			{
				LOG_DEBUG("Couldn't open file");
				error = HttpError(404, "Not Found");
				response = generateErrorResponse(error.message());
				return;
			}
			LOG_DEBUG("File opened");
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
			error = HttpError(500, "Couldn't delete file");
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
		response = generateErrorResponse(error.message());
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

	if (header.getMethod() == "GET" && !error.code()) {
		if (response.size())
		{
			LOG_DEBUG("HttpResponse sending response buffer");
			// sending headers
			ssize_t sentBytes =  send(fds, response.c_str(), response.size(), 0);
			if (sentBytes >= 0)
				response = response.substr(sentBytes);
		} else if (getFile.is_open()) {
			// sending file
			LOG_DEBUG("HttpResponse reading chunk from file");
			getFile.read(chunkedBuffer, 1023);
			if (getFile.gcount() == 0)
			{
				getFile.close();
				response += "0\r\n\r\n";
				return;
			}
			size_t readBytes = getFile.gcount();
			chunkedBuffer[readBytes] = '\0';
			std::stringstream ss;
			ss << std::hex << readBytes;
			response += ss.str();
			response += "\r\n";
			response += chunkedBuffer;
			response += "\r\n";
		} else {
			isFinished = true;
		}
	} else {
		if (response.size() > 0) {
			ssize_t sentBytes =  send(fds, response.c_str(), response.size(), 0);
			LOG_DEBUG_WITH_TAG(response, "response EMPTY?");
			response = response.substr(sentBytes);
		}
		else {
			isFinished = true;
		}
	}
}

bool HttpResponse::finished() {
	return isFinished;
}

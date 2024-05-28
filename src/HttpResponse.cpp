#include "HttpResponse.hpp"
#include <sys/socket.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include "Logger.hpp"
#include "Config.hpp"
#include "Utils.hpp"

bool HttpResponse::isFolder(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool HttpResponse::isFile(const std::string &path)
{
	struct	stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

HttpError HttpResponse::setupGetResponse()
{
	if (isFile(config.getFilePath(header)))
	{
		LOG_DEBUG(config.getFilePath(header).c_str());
		LOG_DEBUG("returning file");
		std::string filePath = config.getFilePath(header);
		LOG_DEBUG(filePath);
		getFile.open(filePath.c_str());
		if (getFile.fail())
			return HttpError(500, "Couldn't open File");
		LOG_DEBUG("File opened");
		response += "200 OK\r\n";
		response += "Connection: close\r\n";
		response += "transfer-encoding: chunked\r\n\r\n";
	}
	else if (isFolder(config.getFilePath(header)))
	{
		LOG_DEBUG(config.getFilePath(header).c_str());
		LOG_DEBUG("Is folder");
		if (config.getDirectiveValue(header, Config::Index).size() != 0)
		{
			LOG_DEBUG("returning index file");
			std::string filePath = config.getFilePath(header) + config.getDirectiveValue(header, Config::Index);
			getFile.open(filePath.c_str());
			if (getFile.fail())
			{
				if (config.getDirectiveValue(header, Config::Listing) == "on")
				{
					LOG_DEBUG("returning listing");
					generateDirListing();
					return HttpError();
				}
				LOG_DEBUG("Couldn't open file");
				return HttpError(404, "Not Found");
			}
			response += "200 OK\r\n";
			response += "Connection: close\r\n";
			response += "transfer-encoding: chunked\r\n\r\n";
		}
		else if (config.getDirectiveValue(header, Config::Listing) == "on")
		{
			LOG_DEBUG("returning listing");
			generateDirListing();
			return HttpError();
		}
		else
		{
			return HttpError(403, "Forbidden");
		}
	}
	else
	{
		return HttpError(404, "Not Found");
	}
	return HttpError();
}

HttpResponse::HttpResponse() : config(Config::getInstance()) {
	isError = false;
	isFinished = false;
}

HttpResponse::HttpResponse(HttpHeader header, int fds) :
		header(header), config(Config::getInstance()), fds(fds) {
	LOG_DEBUG("New HttpResponse created");
	HttpError error = header.getError();
	isError = false;
	isFinished = false;
	if (error.code() != 0)
	{
		response = generateErrorResponse(error);
		isError = true;
		return;
	}
	response = "HTTP/1.1 ";
	host = header.getHost();
	path = header.getPath();

	LOG_DEBUG(config.getDirectiveValue(header, Config::Redir).c_str());
	if (config.getDirectiveValue(header, Config::Redir).size())
	{
		LOG_DEBUG("Redirecting request");
		response += "308 Permanent Redirect\r\n";
		response += "Location: " + config.getDirectiveValue(header, Config::Redir);
		response += "\r\n\r\n";
		return;
	}
	else if (header.getMethod() == "GET" && config.isDirectiveAllowed(header, Config::AllowedMethods, "GET"))
	{
		LOG_DEBUG("GET request");
		error = setupGetResponse();
	}
	else if (header.getMethod() == "POST" && config.isDirectiveAllowed(header, Config::AllowedMethods, "POST"))
	{
		error = HttpError(405, "Method Not Allowed");
		response += "405 Method Not Allowed\r\n";
		response += "Allow: " + config.getDirectiveValue(header, Config::AllowedMethods) + "\r\n";
		response += "Connection: close\r\n\r\n";
	}
	else if (header.getMethod() == "DELETE")
	{
		std::string filePath = config.getFilePath(header);
		LOG_DEBUG("DELETE request");
		if (!config.isDirectiveAllowed(header, Config::AllowedMethods, "DELETE"))
			error = HttpError(405, "Method Not Allowed");
		else if (remove(filePath.c_str()) != 0) {
			error = HttpError(500, "Couldn't delete file");
		}
		else {
			response += "200 OK\r\n\r\n";
			response += "<html><body><h1>File deleted</h1></body></html>\r\n";
		}
	}
	else
	{
		LOG_DEBUG("Request not allowed");
		error = HttpError(405, "Method Not Allowed");
	}

	if (error.code() != 0)
	{
		LOG_DEBUG("ERRROR_________________________________");
		response = generateErrorResponse(error);
		isError = true;
	}
}

std::string HttpResponse::generateErrorResponse(const HttpError &error) {
	std::string error_path = config.getErrorPage(error.code(), header);
	response = "HTTP/1.1 ";
	std::string message = error.message();
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
		std::ifstream file(error_path.c_str());
		if (!file.is_open())
		{
			std::string error_html = "<HTML><body><p><strong>";
			error_html += "500";
			error_html += " </strong>";
			error_html += "Couldn't open error file";
			error_html += "</p></body>";

			response += errCode.str();
			response += " ";
			response += "Couldn't open error file\r\n";
			response += "Connection: close\r\n";
			response += "Content-Type: text/html\r\n";
			response += "Content-Length: ";
			std::stringstream errSize;
			errSize << error_html.size();
			response += errSize.str();
			response += "\r\n\r\n";
			response += error_html;
			return response;
		}
		response += errCode.str() + " ";
		response += message + "\r\n";
		response += "Connection: close\r\n";
		response += "Content-Type: text/html\r\n\r\n";
		response += std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	LOG_DEBUG_WITH_TAG("Generated error response", "HttpResponse::generateErrorResponse");
	return response;
}

int HttpResponse::listDir(std::string dir, std::vector<fileInfo> &files)
{
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(dir.c_str());
	while ((dirp = readdir(dp)) != NULL)
	{
		struct stat fileStat;
		fileInfo fileInf;
		std::string filename;
		if (!dir.empty() && dir[dir.length() - 1] != '/')
			filename = dir + "/" + dirp->d_name;
		else
		 	filename = dir + dirp->d_name;
		LOG_DEBUG_WITH_TAG(filename, "full path");
		if (stat(filename.c_str(), &fileStat) == -1)
		{
			LOG_ERROR("Couldn't get file stats");
		}
		fileInf.name = dirp->d_name;
		fileInf.size = Utils::toString(static_cast<size_t> (fileStat.st_size));
		fileInf.date = std::string(ctime(&fileStat.st_mtime));
		files.push_back(fileInf);
	}
	closedir(dp);
	return 0;
}

void HttpResponse::generateDirListing()
{
	std::string filePath = config.getFilePath(header);
	std::vector<fileInfo> files;
	listDir(filePath, files);
	response += "200 OK\r\n";
	response += "Connection: close\r\n";
	response += "Content-Type: text/html\r\n";
	std::string listing = "<!DOCTYPE html>"
							"<html>"
							"<head>"
							"<style>"
							"#files {"
							"font-family: Arial, Helvetica, sans-serif;"
							"border-collapse: collapse;"
							"width: 100%;"
							"}"
							"#files td, #files th {"
							"border: 1px solid #ddd;"
							"padding: 8px;"
							"}"
							"#files tr:nth-child(even){background-color: #f2f2f2;}"
							"#files tr:hover {background-color: #ddd;}"
							"#files th {"
							"padding-top: 12px;"
							"padding-bottom: 12px;"
							"text-align: left;"
							"background-color: #04AA6D;"
							"color: white;"
							"}"
							"</style>"
							"</head>";
	listing += "<body><h2>Directory listing</h2><table id=\"files\">";
	listing += "<tr><th>Filename</th><th>Size (bytes)</th><th>Time of last data modification</th></tr>";
	for (size_t i = 0; i < files.size(); i++)
	{
		listing += "<tr><td><a href=\"";
		listing += files[i].name;
		listing += "\">";
		listing += files[i].name;
		listing += "</a></td><td>";

		listing += files[i].size;
		listing += "</td><td>";
		listing += files[i].date;
		listing += "</td></tr>";
	}
	listing += "</table></body></html>";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: ";
	response += Utils::toString(listing.size());
	response += "\r\n\r\n";
	response += listing;
	LOG_DEBUG(response.c_str());
}

size_t HttpResponse::readBuffer(const char *buffer) {
	(void) buffer;
	return 0;
}

void HttpResponse::write() {
	LOG_DEBUG("HTTPRESPONSE");
	LOG_DEBUG(response.c_str());
	if (isFinished)
	{
		LOG_INFO("HttpResponse::write isFinished");
		return;
	}

	if (header.getMethod() == "GET" && !isError) {
		if (response.size())
		{
			LOG_DEBUG("HttpResponse sending response buffer");
			// sending headers
			ssize_t sentBytes =  send(fds, response.c_str(), response.size(), MSG_DONTWAIT);
			if (sentBytes >= 0)
				response = response.substr(sentBytes);
		} else if (getFile.is_open()) {
			// sending file
			LOG_DEBUG("HttpResponse reading chunk from file");
			getFile.read(chunkedBuffer, 1023);
			if (getFile.gcount() == 0)
			{
				LOG_DEBUG("HttpResponse file read finished");
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
			response += std::string(chunkedBuffer, readBytes);
			response += "\r\n";
		} else {
			isFinished = true;
		}
	} else {
		if (response.size() > 0) {
			ssize_t sentBytes =  send(fds, response.c_str(), response.size(), 0);
			if (sentBytes >= 0)
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

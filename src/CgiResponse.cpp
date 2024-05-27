#include <CgiResponse.hpp>

CgiResponse::CgiResponse(const std::string &cgiBuffer, const int &fd) :
		_state(NO_RESPONSE),
		_cgiBuffer(cgiBuffer),
		_isResponseBodyPresent(false),
		_responseHeaderSize(0),
		_responseBodySize(0),
		_headerBytesSend(0),
		_bodyBytesSend(0),
		_fd(fd),
		_headerSent(false),
		_bodySent(false),
		_isInternalRedirect(false) {}

CgiResponse::~CgiResponse() {}

int CgiResponse::sendResponse() {
	if (parseResponse() < 0) {
		return -1;
	}
	if (_headerSent == false && _state != LOCAL_REDIRECT) {
		if (sendHeader() < 0) {
			return -1;
		}
		return 0;
	}
	switch (_state) {
		case DOCUMENT_RESPONSE:
			if (_bodySent == false) {
				if (sendBody() == -1) {
					return -1;
				}
			} else {
				_state = FINISHED;
			}
			break;
		case LOCAL_REDIRECT:
			_isInternalRedirect = true;
			_state = FINISHED;
			break;
		case CLIENT_REDIRECT:
			_state = FINISHED;
			break;
		case CLIENT_REDIRECT_WITH_BODY:
			if (_bodySent == false) {
				if (sendBody() == -1) {
					return -1;
				}
			} else {
				_state = FINISHED;
			}
			break;
		case FINISHED:
			LOG_ERROR_WITH_TAG("Response already sent (THIS SHOULD NOT TRIGGER)", "CGI");
		default:
			break;
	}
	return 0;
}

// Checks if the status code is valid
bool CgiResponse::isValidStatusCode(const std::string &statusCode) const {
	if (statusCode.size() < 3) {
		return false;
	}
	if (statusCode[0] < '1' || statusCode[0] > '5' || !isdigit(statusCode[1]) || !isdigit(statusCode[2])) {
		return false;
	}
	if (statusCode.size() > 3 && !isspace(statusCode[3])) {
		return false;
	}
	return true;
}

// Checks if the content-type value is valid
bool CgiResponse::isValidContentType(const std::string &line) const {
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos || colonPos == 0) {
		// No colon found or the colon is the first character
		return false;
	}
	if (line.substr(0, colonPos) != "Content-Type") {
		return false;
	}
	if (line.substr(colonPos + 1, 1) != " ") {
		return false;
	}

	std::string mimeType = line.substr(colonPos + 2);
	size_t slashPos = mimeType.find('/');
	if (slashPos == std::string::npos || slashPos == 0 || slashPos == mimeType.length() - 1) {
		// No slash found, or the slash is the first or last character
		return false;
	}

	// Check that the type and subtype consist only of alphanumeric characters and hyphens
	for (size_t i = 0; i < mimeType.length(); ++i) {
		if (i != slashPos && !std::isalnum(mimeType[i]) && mimeType[i] != '-') {
			return false;
		}
	}
	return true;
}

// Adds a header field to the _responseHeaders map and returns -1 on error
int CgiResponse::addHeaderField(const std::string &line) {
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos || colonPos == 0) {
		return -1;
	}
	std::string key = Utils::toLowerString(line.substr(0, colonPos));
	std::string value = line.substr(colonPos + 1);
	value = value.substr(value.find_first_not_of(' '), value.find_last_not_of(' ') + 1);
	// Don't add the header field if the value is empty
	if (value.empty()) {
		return 0;
	}
	// Check if a key that is only allowed once already exists and if it does, return an error
	if (key.compare(0, 6, "status") == 0 ||
			key.compare(0, 13, "content-type") == 0 ||
			key.compare(0, 14, "content-length") == 0 ||
			key.compare(0, 8, "location") == 0) {
		if (_responseHeaders.find(key) != _responseHeaders.end()) {
			return -1;
		}
	} else {
		// If the key already exists, append the value to the existing value
		if (_responseHeaders.find(key) != _responseHeaders.end()) {
			if (!value.empty()) {
				_responseHeaders[key] += ", " + value;
			}
		} else {
			_responseHeaders[key] = value;
		}
	}
	return 0;
}

// Checks if a header field is present case-insensitively
bool CgiResponse::isHeaderFieldPresent(const std::string &key) const {
	std::string lowerCaseKey = Utils::toLowerString(key);
	if (_responseHeaders.find(lowerCaseKey) != _responseHeaders.end()) {
		return true;
	}
	return false;
}

// Reads the header of the return value into the _responseHeaders map, returns -1 on error
int CgiResponse::parseHeader() {
	if (_cgiBuffer.empty()) {
		return 0;
		LOG_ERROR_WITH_TAG("Empty CGI return buffer", "CGI");
	}
	size_t pos = _cgiBuffer.find("\n\n");
	if (pos == std::string::npos) {
		LOG_ERROR_WITH_TAG("Invalid return value (Missing: \n\n)", "CGI");
		return 0;
	}
	std::string header = _cgiBuffer.substr(0, pos);
	std::istringstream headerStream(header);
	std::string line;
	std::getline(headerStream, line);
	while (line.empty() == false) {
		if (addHeaderField(line) == -1) {
			return 0;
		}
		std::getline(headerStream, line);
	}

	return pos + 2;
}

// Checks if a path is a valid HTTP path
bool isValidHttpPath(const std::string &path) {
	// Check if path starts with "http://" or "/"
	if (path.substr(0, 7) != "http://" && path[0] != '/') {
		return false;
	}

	// Check for invalid characters
	const std::string valid_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"-._~:/?#[]@!$&'()*+,;=%";

	if (path.find_first_not_of(valid_chars) != std::string::npos) {
		return false;
	}

	return true;
}

// Checks if a path is a http:// path
bool CgiResponse::isUrlPath(const std::string &path) const {
	if (path.find("http://") == 0) {
		return true;
	}
	return false;
}

// Checks if a path is a local path
bool CgiResponse::isLocalPath(const std::string &path) const {
	if (path.find("/") == 0) {
		return true;
	}
	return false;
}

std::string CgiResponse::createResponseHeader() {
	if (_state == DOCUMENT_RESPONSE) {
		if (_responseHeaders.find("status") != _responseHeaders.end()) {
			_responseHeader = "HTTP/1.1 " + _responseHeaders["status"] + "\r\n";
		} else {
			_responseHeader = "HTTP/1.1 200 OK\r\n";
		}
	} else if (_state == CLIENT_REDIRECT) {	
		_responseHeader = "HTTP/1.1 302 Found\r\n";
	} else if (_state == CLIENT_REDIRECT_WITH_BODY) {
		_responseHeader = "HTTP/1.1 302 Found\r\n";
	}
	for (std::map<std::string, std::string>::iterator header = _responseHeaders.begin(); header != _responseHeaders.end(); ++header) {
		_responseHeader += header->first + ": " + header->second + "\r\n";
	}
	_responseHeader += "\r\n";
	return _responseHeader;
}

bool CgiResponse::isFinished() const {
	return _state == FINISHED;
}

bool CgiResponse::isInternalRedirect() const {
	return _isInternalRedirect;
}

std::string CgiResponse::getInternalRedirectLocation() const {
	if (_responseHeaders.count("location") > 0) {
		return _responseHeaders.at("location");
	} else {
		return "";
	}
}

int CgiResponse::sendHeader() {
	ssize_t bytesSent = send(_fd, _responseHeader.c_str() + _headerBytesSend, _responseHeader.size() - _headerBytesSend, MSG_DONTWAIT);
	if (bytesSent < 0) {
		LOG_ERROR_WITH_TAG("Failed to send response header", "CGI");
		return -1;
	}
	_headerBytesSend += bytesSent;
	if (_headerBytesSend == _responseHeader.size()) {
		_headerSent = true;
	}
	return 0;
}

int CgiResponse::sendBody() {
	ssize_t bytesSent = send(_fd, _responseHeader.c_str() + _headerBytesSend + _bodyBytesSend, _responseHeader.size() - _headerBytesSend  - _bodyBytesSend, MSG_DONTWAIT);
	if (bytesSent < 0) {
		LOG_ERROR_WITH_TAG("Failed to send response body", "CGI");
		return -1;
	}
	_bodyBytesSend += bytesSent;
	if (_bodyBytesSend == _responseHeader.size()) {
		_bodySent = true;
	}
	return 0;
}

// https://datatracker.ietf.org/doc/html/rfc3875.html#section-6.2
// Set the response state based on the headers
int CgiResponse::setState() {
	bool hasLocation = isHeaderFieldPresent("location");
	bool hasContentType = isHeaderFieldPresent("content-type");
	bool hasStatus = isHeaderFieldPresent("status");

	if (hasLocation) {
		if (!isValidHttpPath(_responseHeaders["location"])) {
			return -1;
		}
		if (isUrlPath(_responseHeaders["location"])) {
			if (_responseHeaders.size() == 1 && _isResponseBodyPresent == false) {
				_state = CLIENT_REDIRECT;
				return 0;
			} else if (hasStatus && hasContentType) {
				if (_responseHeaders["status"] == "302 Found") {
					_state = CLIENT_REDIRECT_WITH_BODY;
				}
			} else {
				return -1;
			}
		} else if (isLocalPath(_responseHeaders["location"])) {
			if (_responseHeaders.size() == 1 && _isResponseBodyPresent == false) {
				_state = LOCAL_REDIRECT;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	} else if (isHeaderFieldPresent("content-type")) {
		_state = DOCUMENT_RESPONSE;
	} else {
		return -1;
	}
	return 0;
}

int CgiResponse::parseResponse() {
	// Check if the response has already been parsed
	if (_responseHeaders.size() > 0) {
		return 0;
	}
	_responseHeaderSize = parseHeader();
	if (_responseHeaderSize == 0) {
		return -1;
	}

	// Check if a body is present
	if (_responseHeaderSize < _cgiBuffer.size()) {
		_responseBodySize = _cgiBuffer.size() - _responseHeaderSize;
		_isResponseBodyPresent = true;
	}

	if (setState() < 0) {
		return -1;
	}

	return 0;
}

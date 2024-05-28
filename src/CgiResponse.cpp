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
		LOG_ERROR_WITH_TAG("parse Failed", "CGI Response");
		return -1;
	}
	if (_headerSent == false && _state != LOCAL_REDIRECT) {
		if (sendHeader() < 0) {
			LOG_ERROR_WITH_TAG("sendHeader CGI", "CGI Response");
			return -1;
		}
		LOG_DEBUG_WITH_TAG("send okay", "CGI Response");
		return 0;
	}
	LOG_DEBUG_WITH_TAG("going for the switch", "CGI Response");
	if (!_isResponseBodyPresent) {
		_bodySent = true;
		LOG_DEBUG_WITH_TAG("no body present", "CGI Response");
	}
	switch (_state) {
		case DOCUMENT_RESPONSE:
			std::cout << "Response body size: " << _responseBodySize << std::endl;
			if (_bodySent == false) {
				if (sendBody() == -1) {
					LOG_DEBUG_WITH_TAG("send body failed", "CGI Response");
					return -1;
				}
			} else {
				LOG_DEBUG_WITH_TAG("send body finished", "CGI Response");
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
			LOG_ERROR_WITH_TAG("Response already sent (THIS SHOULD NOT TRIGGER)", "CGI Response");
			break;
		default:
			LOG_DEBUG_WITH_TAG("breakyboy", "CGI Response");
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
	LOG_DEBUG_WITH_TAG("Status Code is valid", "CGI Response");
	return true;
}

// Checks if the content-type value is valid
bool CgiResponse::isValidContentType(const std::string &line) const {
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos || colonPos == 0) {
		// No colon found or the colon is the first character
		return false;
	}
	if (Utils::toLowerString(line.substr(0, colonPos)) != "content-type") {
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
	// std::cout << "toLower: " << line.substr(0, colonPos) << " colonPos: " << colonPos << std::endl;
	std::string key = Utils::toLowerString(line.substr(0, colonPos));
	std::string value = line.substr(colonPos + 1);
	value = value.substr(value.find_first_not_of(' '), value.find_last_not_of(' ') + 1);
	// Don't add the header field if the value is empty
	// std::cout << "value: " << value << " key: " << key << std::endl;
	if (value.empty()) {
		return 0;
	}
	// Check if a key that is only allowed once already exists and if it does, return an error
	if (key.compare(0, 6, "status") == 0 ||
			key.compare(0, 12, "content-type") == 0 ||
			key.compare(0, 14, "content-length") == 0 ||
			key.compare(0, 8, "location") == 0) {
		if (_responseHeaders.find(key) != _responseHeaders.end()) {
			return -1;
		}
	}
	if (key.compare(0, 6, "status") == 0 && !isValidStatusCode(value)) {
		return -1;
	}
	// If the key already exists, append the value to the existing value
	if (_responseHeaders.find(key) != _responseHeaders.end()) {
		if (!value.empty()) {
			_responseHeaders[key] += ", " + value;
		}
	} else {
		_responseHeaders[key] = value;
	}

	return 0;
}

// Checks if a header field is present case-insensitively
bool CgiResponse::isHeaderFieldPresent(const std::string &key) const {
	std::string lowerCaseKey = Utils::toLowerString(key);
	// std::cout << "key: " << lowerCaseKey << std::endl;
	std::map<std::string, std::string>::const_iterator it;
	// for (it = this->_responseHeaders.begin(); it != this->_responseHeaders.end(); ++it) {
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// }
	if (_responseHeaders.find(lowerCaseKey) != _responseHeaders.end()) {
		LOG_DEBUG_WITH_TAG("Header field is present", "CGI Response");
		return true;
	}
	LOG_DEBUG_WITH_TAG("Header field is not present", "CGI Response");
	return false;
}


// Reads the header of the return value into the _responseHeaders map, returns 0 on error
int CgiResponse::parseHeader() {
	if (_cgiBuffer.empty()) {
		return 0;
		LOG_DEBUG_WITH_TAG("Empty CGI return buffer", "CGI Response");
	}
	size_t pos = _cgiBuffer.find("\r\n\r\n");
	if (pos == std::string::npos) {
		LOG_DEBUG_WITH_TAG("Invalid return value (Missing: \\r\\n\\r\\n)", "CGI Response");
		return 0;
	}
	std::string header = _cgiBuffer.substr(0, pos);
	std::istringstream headerStream(header);
	std::string line;
	std::getline(headerStream, line);
	while (line.empty() == false) {
		if (line[line.size() - 1] == '\r') {
			line = line.substr(0, line.size() - 1);
		}
		if (addHeaderField(line) == -1) {
			LOG_DEBUG_WITH_TAG(line, "CGI LINE");
			return 0;
		}
		if (!std::getline(headerStream, line))
			break;
	}

	return pos + 4;
}

// Checks if a path is a valid HTTP path
bool isValidHttpPath(const std::string &path) {
	// Check if path starts with "http://" or "/"
	if (path.substr(0, 7) != "http://" && path.substr(0, 8) != "https://" && path[0] != '/') {
		LOG_DEBUG_WITH_TAG("invalid path", "CGI Response");
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
	LOG_DEBUG_WITH_TAG("valid path", "CGI Response");
	return true;
}

// Checks if a path is a http:// path
bool CgiResponse::isUrlPath(const std::string &path) const {
	if (path.find("http://") == 0 || path.find("https://") == 0) {
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

void CgiResponse::createResponseHeader() {
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
	std::cout << "bytesSent Header: " << bytesSent << std::endl;
	if (bytesSent < 0) {
		LOG_ERROR_WITH_TAG("Failed to send response header", "CGI Response");
		return -1;
	}
	_headerBytesSend += bytesSent;
	if (_headerBytesSend == _responseHeader.size()) {
		_headerSent = true;
	}
	return 0;
}

int CgiResponse::sendBody() {
	ssize_t bytesSent = send(_fd, _cgiBuffer.c_str() + _responseHeaderSize + _bodyBytesSend, _responseBodySize - _bodyBytesSend, MSG_DONTWAIT);
	std::cout << "_cgiBuffer.c_str(): " << _cgiBuffer.c_str() << std::endl;
	std::string test(_cgiBuffer.c_str() + _responseHeaderSize + _bodyBytesSend, _responseHeader.size() - _responseHeaderSize - _bodyBytesSend);
	std::cout << "test string: " << test << std::endl;
	std::cout << "_responseHeaderSize: " << _responseHeaderSize << " _bodyBytesSend: " << _bodyBytesSend << " _responseBodySize.size(): " << _responseBodySize  << " _responseHeaderSize: " << _responseHeaderSize << " _bodyBytesSend: " << _bodyBytesSend << std::endl;
	std::cout << "bytesSent body: " << bytesSent << std::endl;
	if (bytesSent < 0) {
		LOG_ERROR_WITH_TAG("Failed to send response body", "CGI Response");
		return -1;
	}
	_bodyBytesSend += bytesSent;
	if (_bodyBytesSend == _responseBodySize) {
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

	// std::cout << "cgibuffer: " << _cgiBuffer << std::endl;
	if (hasLocation) {
		if (!isValidHttpPath(_responseHeaders["location"])) {
			LOG_DEBUG_WITH_TAG("no valid http path", "CGI Response");
			return -1;
		}
		if (isUrlPath(_responseHeaders["location"])) {
			if (_responseHeaders.size() == 1 && _isResponseBodyPresent == false) {
				_state = CLIENT_REDIRECT;
				LOG_DEBUG_WITH_TAG("client redirect", "CGI Response");
				return 0;
			} else if (hasStatus && hasContentType) {
				if (_responseHeaders["status"] == "302 Found") {
					_state = CLIENT_REDIRECT_WITH_BODY;
					LOG_DEBUG_WITH_TAG("client redirect with body", "CGI Response");
				}
			} else {
				LOG_DEBUG_WITH_TAG("wrong external redirect", "CGI Response");
				return -1;
			}
		} else if (isLocalPath(_responseHeaders["location"])) {
			if (_responseHeaders.size() == 1 && _isResponseBodyPresent == false) {
				_state = LOCAL_REDIRECT;
				LOG_DEBUG_WITH_TAG("internal redirect", "CGI Response");
			} else {
				LOG_DEBUG_WITH_TAG("wrong format internal redirect", "CGI Response");
				return -1;
			}
		} else {
			LOG_DEBUG_WITH_TAG("no valid location path", "CGI Response");
			return -1;
		}
	} else if (isHeaderFieldPresent("content-type")) {
		LOG_DEBUG_WITH_TAG("document response", "CGI Response");
		_state = DOCUMENT_RESPONSE;
	} else {
		LOG_DEBUG_WITH_TAG("fuck no", "CGI Response");
		return -1;
	}
	return 0;
}

int CgiResponse::parseResponse() {
	// Check if the response has already been parsed
	if (_responseHeaders.size() > 0) {
		LOG_DEBUG_WITH_TAG("already parsed", "CGI Response");
		return 0;
	}
	LOG_DEBUG("CGI BUFFER");
	std::cout << _cgiBuffer << std::endl;
	_responseHeaderSize = parseHeader();
	if (_responseHeaderSize == 0) {
		LOG_DEBUG_WITH_TAG("parse Header failed", "CGI Response");
		return -1;
	}

	// Check if a body is present
	if (_responseHeaderSize < _cgiBuffer.size()) {
		_responseBodySize = _cgiBuffer.size() - _responseHeaderSize;
		_isResponseBodyPresent = true;
		LOG_DEBUG_WITH_TAG("response body is present", "CGI Response");
	}
	if (setState() < 0) {
		LOG_DEBUG_WITH_TAG("setState failed", "CGI Response");
		return -1;
	}
	createResponseHeader();
	return 0;
}


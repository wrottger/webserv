#include "Client.hpp"
#include "EventHandler.hpp"

 // TODO: Delete this function
std::string createTestResponse() {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
							   "<body><h1>Keine pull request approve fuer Freddy!</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string httpResponse = "HTTP/1.1 200 OK\r\n"
							   "Content-Type: text/html; charset=UTF-8\r\n"
							   "Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return httpResponse;
}

Client::Client() {}

Client::Client(int fd, std::string ip):
		_lastModified(0),
		_fd(fd),
		_canBeDeleted(false),
		_state(READING_HEADER),
		_ip(ip) {
	LOG_DEBUG(ip);
	_headerObject = new HttpHeader;
	updateTime();
	httpResponse = NULL;
}

Client::~Client() {
	if (httpResponse != NULL) {
		delete httpResponse;
	}
	delete _headerObject;
}

int Client::getFd() const {
	return _fd;
}

// MAIN LOOP for processing client requests
void Client::process(uint32_t events) {
	switch (_state) {
		case READING_HEADER:
			if (events & EPOLLIN) {
				readFromClient();
				if (isHeaderComplete() && _headerObject->isError() == false) {
					if (Config::getInstance()->isCGIAllowed(_headerObject->getPath(), _headerObject->getHost())) {
						if (_headerObject->getMethod() == "POST") {
							_state = READING_BODY;
							LOG_DEBUG("set State: Reading body");
						}
						else if (_headerObject->getMethod() == "GET") {
							_state = WAITING_FOR_CGI;
							LOG_DEBUG("set State: Waiting for CGI");
							// processCGI(withoutBody);
						}

					} else {
					_state = SENDING_RESPONSE;
					LOG_DEBUG("set State: Sending response");
					httpResponse = new HttpResponse(*_headerObject, _fd);
					// TODO: Copy the rest of the buffer to the response object
					// TODO: Create a response object
					}
				}
			}
			break;
		case READING_BODY:
				if (_headerObject->isInHeader("transfer-encoding")) {
					if (_headerObject->getHeader("transer-encoding").find("chunked") != std::string::npos) {
						// get chunked stuff;
						// size_t bodySize = _headerObject->getHeader("content-length");
					} else {
						// read full body;
					}
				}
				if (_headerObject->getHeader("transer-encoding").find("chunked")) {
					// get chunked stuff;
					// size_t bodySize = _headerObject->getHeader("content-length");
				} else {
					// read full body;
				}
				// if body isBodyComplete
				// processCGI(body);
				// _state = WAITING_FOR_CGI;
			break;
		case WAITING_FOR_CGI:
			// processCgi();
			// if (cgi->isFinished()) {
			// 	_state = SENDING_RESPONSE;
			// }
			break;
		case SENDING_RESPONSE:
			// sendResponse();
			// if (response->isSent()) {
				// _state = FINISHED;
			// }
			if (events & EPOLLOUT) {
				if (httpResponse->finished()) {
					LOG_DEBUG("HttpResponse::finished");
					_state = FINISHED;
				} else {
					httpResponse->write();
				}
			}
			break;
		case FINISHED:
			_canBeDeleted = true;
			break;
	}
}

// Returns the last time the client was modified
std::time_t Client::getLastModified() const {
	return _lastModified;
}

// Updates the last modified time to the current time
void Client::updateTime() {
	_lastModified = std::time(0);
}

// Returns true if the header is complete
bool Client::isHeaderComplete() const {
	return _headerObject->isComplete();
}

// Returns true if the client can be deleted
bool Client::canBeDeleted() const {
	return _canBeDeleted;
}

HttpHeader *Client::getHeaderObject() const {
	return _headerObject;
}

// Returns true if the client has exceeded the timeout
bool Client::isTimeouted() const {
	return (std::time(0) - _lastModified) > CLIENT_TIMEOUT;
}

Client &Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

// Reads from the client and parses the buffer
void Client::readFromClient() {
	char buffer[BUFFER_SIZE + 1] = { 0 };
	ssize_t bytes_received = read(_fd, buffer, BUFFER_SIZE);
	// The client has closed the connection
	if (bytes_received <= 0) {
		_canBeDeleted = true;
		LOG_DEBUG("Client closed connection");
	} else {
		buffer[bytes_received] = '\0';
		size_t parsedSize = _headerObject->parseBuffer(buffer);
		if (parsedSize != BUFFER_SIZE) {
			// save rest to bodybuffer
			_bodyBuffer = std::string(&buffer[parsedSize]);
		}
		std::string bufferDebug(buffer); //TODO: DELETE DEBUG
		LOG_BUFFER(bufferDebug);		//TODO: DELETE DEBUG
	}
}

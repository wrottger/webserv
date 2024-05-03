#include "Client.hpp"
#include "EventHandler.hpp"

 // TODO: Delete this function
std::string createTestResponse() {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
							   "<body><h1>Hello, World!</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string httpResponse = "HTTP/1.1 200 OK\r\n"
							   "Content-Type: text/html; charset=UTF-8\r\n"
							   "Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return httpResponse;
}

Client::Client() {}

Client::Client(int fd):
		_lastModified(0),
		_fd(fd),
		_canBeDeleted(false),
		_state(READING_HEADER) {
	_headerObject = new HttpHeader;
	updateTime();
}

Client::~Client() {
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
				if (isHeaderComplete()) {
					_state = SENDING_RESPONSE;
					// TODO: Copy the rest of the buffer to the response object
					// TODO: Create a response object
					
				}
			}
			break;
		case READING_BODY:
			// responeObject->parseBuffer(buffer);
			// if (responseObject->isBodyisComplete()) {
				// if (responseObject->isCgi()) {
					// _cgi_object = new Cgi(responseObject->getBody(), _headerObject);
					// _state = WAITING_FOR_CGI;
				// } else {
					// _state = SENDING_RESPONSE;
				// }
			// }
			break;
		case WAITING_FOR_CGI:
			// processCgi();
			// if (cgi->isFinished()) {
				// _state = SENDING_RESPONSE;
			// }
			break;
		case SENDING_RESPONSE:
			// sendResponse();
			// if (response->isSent()) {
				// _state = FINISHED;
			// }
			if (events & EPOLLOUT) {
				if (isHeaderComplete()) {
					std::string httpResponse = createTestResponse();
					if (send(_fd, httpResponse.c_str(), httpResponse.size(), 0) == -1) {
						LOG_ERROR_WITH_TAG("send failed", "Client");
					}
					_state = FINISHED;
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

// TODO: Rename to parseHeaderBuffer or something similar
void Client::parseBuffer(const char *buffer) {
	_headerObject->parseBuffer(buffer);
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
	if (bytes_received == 0) { // TODO: Merge with -1 for readability
		_canBeDeleted = true;
		LOG_DEBUG("Client connection closes 0");
	} else if (bytes_received == -1) {
		_canBeDeleted = true;
		LOG_DEBUG("Client connection closes -1");
	} else {
		buffer[bytes_received] = '\0';
		parseBuffer(buffer);
		std::string bufferDebug(buffer); //TODO: DELETE DEBUG
		LOG_BUFFER(bufferDebug);
	}
}

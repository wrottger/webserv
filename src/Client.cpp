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
				if (isHeaderComplete()) {
					_state = SENDING_RESPONSE;
					httpResponse = new HttpResponse(*_headerObject, _fd);
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

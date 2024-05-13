#include "Client.hpp"
#include "EventHandler.hpp"

 // TODO: Delete this function
std::string createTestResponse() {
	std::string responseBody = "<!DOCTYPE html><html><head><title>Hello World</title></head>"
							   "<body><h1>Keine pull request approve fuer Freddy!</h1></body></html>";

	std::ostringstream oss;
	oss << responseBody.size();

	std::string _responseHttp = "HTTP/1.1 200 OK\r\n"
							   "Content-Type: text/html; charset=UTF-8\r\n"
							   "Content-Length: " +
			oss.str() + "\r\n\r\n" + responseBody;
	return _responseHttp;
}

Client::Client() {}

Client::Client(int fd, std::string ip):
		_header(NULL),
		_responseHttp(NULL),
		_responseCgi(NULL),
		_lastModified(0),
		_fd(fd),
		_canBeDeleted(false),
		_state(READING_HEADER),
		_ip(ip),
		_bodyBuffer("") {
	_header = new HttpHeader;
	updateTime();
	_responseHttp = NULL;
}

Client::~Client() {
	LOG_DEBUG_WITH_TAG("Client destructor called", "Client");
	if (_responseHttp != NULL) {
		delete _responseHttp;
	}
	if (_responseCgi != NULL) {
		delete _responseCgi;
	}
	delete _header;
}

int Client::getFd() {
	return _fd;
}

HttpHeader &Client::getHeaderObject() {
	return *_header;
}

// MAIN LOOP for processing client requests
void Client::process(EventsData *eventData) {
	switch (_state) {
		case READING_HEADER:
			if (eventData->eventMask & EPOLLIN) {
				readFromClient();
				if (isHeaderComplete() && _header->isError() == false) {
					if (Config::getInstance()->isCGIAllowed(_header->getPath(), _header->getHost())) {
						_state = CGI_RESPONSE;
					} else {
						_state = NORMAL_RESPONSE;
						LOG_DEBUG("set State: NORMAL_RESPONSE");
						_responseHttp = new HttpResponse(*_header, _fd);
					}
				}
			}
			break;
		case CGI_RESPONSE:
			if (_responseCgi == NULL) {
				_responseCgi = new Cgi(this);
			}
			_responseCgi->process(eventData);
			if (_responseCgi->isFinished()) {
				_state = FINISHED;
			}
			break;
		case NORMAL_RESPONSE:
			if (eventData->eventMask & EPOLLOUT) {
				if (_responseHttp->finished()) {
					LOG_DEBUG("HttpResponse::finished");
					_state = FINISHED;
				} else {
					_responseHttp->write();
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
	return _header->isComplete();
}

// Returns true if the client can be deleted
bool Client::isDeletable() const {
	return _canBeDeleted;
}

std::string &Client::getBodyBuffer() {
	return _bodyBuffer;
}

const std::string& Client::getIp() const{
	return _ip;
}

bool Client::hasCgi() const {
	return _responseCgi != NULL;
}

Cgi *Client::getCgi() {
	return _responseCgi;
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
		size_t parsedSize = _header->parseBuffer(buffer);
		if (parsedSize < static_cast<size_t>(bytes_received)) {
			// save rest to bodybuffer
			_bodyBuffer = std::string(&buffer[parsedSize]);
		}
		std::string bufferDebug(buffer); //TODO: DELETE DEBUG
		LOG_BUFFER(bufferDebug);		//TODO: DELETE DEBUG
	}
}

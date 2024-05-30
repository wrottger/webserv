#include "Client.hpp"
#include "EventHandler.hpp"

Client::Client() {}

Client::Client(int fd, std::string ip):
		_responseHttp(NULL),
		_cgi(NULL),
		_lastModified(0),
		_fd(fd),
		_canBeDeleted(false),
		_state(READING_HEADER),
		_ip(ip) {
	updateTime();
}

Client::~Client() {
	LOG_DEBUG_WITH_TAG("Client destructor called", "Client");
	delete _responseHttp;
	delete _cgi;
}

int Client::getFd() {
	return _fd;
}

HttpHeader &Client::getHeaderObject() {
	return _header;
}

// MAIN LOOP for processing client requests
void Client::process(EventsData *eventData) {
	switch (_state) {
		case READING_HEADER:
			if (eventData->eventMask & EPOLLIN) {
				if (!isHeaderComplete()) {
					readFromClient();
				}
				if (isHeaderComplete()) {
					if (!_header.isError() && Config::getInstance().isCGIAllowed(_header)) {
						_state = CGI_RESPONSE;
					} else {
						_state = NORMAL_RESPONSE;
						LOG_DEBUG("set State: NORMAL_RESPONSE");
						_responseHttp = new HttpResponse(_header, _fd);
					}
				}
			}
			break;
		case CGI_RESPONSE:
			if (_cgi == NULL) {
				_cgi = new Cgi(this);
				LOG_DEBUG("Creating new Cgi object");
			}
			_cgi->process(eventData);
			if (_cgi->isFinished()) {
				if (_cgi->isInternalRedirect()) {
					_header.setPath(_cgi->getInternalRedirectLocation());
					redirectReset();
					if (Config::getInstance().isCGIAllowed(_header)) {
						_state = CGI_RESPONSE;
					} else {
						_state = NORMAL_RESPONSE;
						LOG_DEBUG("set State: NORMAL_RESPONSE");
						_responseHttp = new HttpResponse(_header, _fd);
					}
				} else {
					_state = FINISHED;
				}
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
	return _header.isComplete();
}

// Returns true if the client can be deleted
bool Client::isDeletable() const {
	return _canBeDeleted;
}

// Returns the body buffer
std::vector<char> &Client::getBodyBuffer() {
	return _bodyBuffer;
}

// Returns the IP of the client
const std::string& Client::getIp() const{
	return _ip;
}

// Returns true if the client has a CGI response
bool Client::hasCgi() const {
	return _cgi != NULL;
}

// Returns the CGI response
Cgi *Client::getCgi() {
	return _cgi;
}

// Returns true if the client has exceeded the timeout in READING_HEADER state
bool Client::isTimeouted() const {
	if (((std::time(0) - _lastModified) > CLIENT_TIMEOUT) && _state == READING_HEADER)
		return true;
	return false;
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
		size_t parsedSize = _header.parseBuffer(buffer);
		if (parsedSize < static_cast<size_t>(bytes_received)) {
			// save rest to bodybuffer
			for (; parsedSize < static_cast<size_t>(bytes_received); parsedSize++)
				_bodyBuffer.push_back(buffer[parsedSize]);
		}
	}
}

void Client::redirectReset() {
	if (_cgi) {
		EventHandler::getInstance().addToCleanUpList(_cgi->getEventData());
		delete _cgi;
	}
	delete _responseHttp;
	_responseHttp = NULL;
	_cgi = NULL;
	_bodyBuffer.clear();
}

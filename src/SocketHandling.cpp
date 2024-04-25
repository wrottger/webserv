#include "SocketHandling.hpp"
#include "Config.hpp"
#include <algorithm>

SocketHandling::SocketHandling(std::vector<Config::ServerBlock> &config) : _config(config)
{
	size_t serverCount = config.size();
    std::vector<int> ports;
	ports = Config::getPorts(config);
	try {
		for (size_t i = 0; i < serverCount; i++) {
			setUpSocket(ports[i]);
		}
		setUpEpoll();
	} catch (const std::runtime_error& e) {
		for (size_t i = 0; i < _openFds.size(); i++) {
			close(_openFds[i]);
		}
		std::cerr << e.what() << std::endl;
	}
}

SocketHandling::SocketHandling(SocketHandling const &other) : _config(other._config){}

SocketHandling SocketHandling::operator=(SocketHandling const &other) {
	(void)other;
	return *this;
}

void SocketHandling::setUpSocket(int port)
{
	int opt = 1, socketFd;
	struct sockaddr_in addr;
	// socklen_t addrlen = sizeof(addr);

	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		throw std::runtime_error("SetUpSocket: socket failed.");
	}

	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		throw std::runtime_error("SetUpSocket: Setsockopt failed.");
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	// Forcefully attaching socket to the port
	if (bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		throw std::runtime_error("SetUpSocket: Bind failed.");
	}
	if (listen(socketFd, 128) < 0) {
		throw std::runtime_error("SetUpSocket: Listen failed.");
	}
	_openFds.push_back(socketFd);
}

int SocketHandling::getEpollFd()
{
	return _epollFd;
}

std::vector<int> SocketHandling::getOpenFds()
{
	return _openFds;
}

SocketHandling::~SocketHandling() {
	for (size_t i = 0; i < _openFds.size(); i++) {
		close(_openFds[i]);
	}
}

void SocketHandling::setUpEpoll() {
	struct epoll_event ev;

	_epollFd = epoll_create(1);
	if (_epollFd == -1) {
		throw std::runtime_error("SetUpEpoll: epoll_create failed.");
	}

	ev.events = EPOLLIN;
	// ev.events = EPOLLIN | EPOLLOUT;

	size_t openSocketCount = _openFds.size();
	for (size_t i = 0; i < openSocketCount; i++) {
		ev.data.fd = _openFds[i];
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _openFds[i], &ev) == -1) {
			throw std::runtime_error("SetUpEpoll: epoll_ctl failed.");
		}
	}
}

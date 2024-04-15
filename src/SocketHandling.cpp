#include "SocketHandling.hpp"

SocketHandling::SocketHandling(std::vector<configObject> &config) : config(config)
{
	size_t serverCount = config[0].getServerCount();
	std::vector<int> ports = config[0].getPorts();

	try {
		for (size_t i = 0; i < serverCount; i++) {
			setUpSocket(ports[i]);
		}
	} catch (const std::runtime_error& e) {
		for (size_t i = 0; i < openFds.size(); i++) {
			close(openFds[i]);
		}
		std::cerr << e.what() << std::endl;
	}
}

SocketHandling::SocketHandling(SocketHandling const &source) : config(config) {}

SocketHandling SocketHandling::operator=(SocketHandling const &source) {}

void SocketHandling::setUpSocket(int port)
{
	int opt = 1, socketFd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

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
	openFds.push_back(socketFd);
}


SocketHandling::~SocketHandling()
{
	for (size_t i = 0; i < openFds.size(); i++) {
		close(openFds[i]);
	}
}

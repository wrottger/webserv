/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnebatz <dnebatz@student.42wolfsburg.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/03 21:42:08 by dnebatz           #+#    #+#             */
/*   Updated: 2024/04/17 11:46:55 by dnebatz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstring> // For memset
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // For close()
#include <sstream>
#include <fstream>
#include <sys/epoll.h>  // für epoll_create1()

#define MAX_EVENTS 10

int main()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd, i = 0;
	int opt = 1;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 310\r\n\r\n";
	std::string fullhttpResponse;
	std::stringstream readFileStream;
	std::string fileContent;
	char buffer[1024] = {0};

	
	/* Code to set up listening socket, 'listen_sock',
		(socket(), bind(), listen()) omitted. */
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		std::cerr << "socket failed: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}

	// Setting options for more than one sockets for same port
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		std::cerr << "setsockopt" << std::endl;
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	
	// Forcefully attaching socket to the port
	if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		std::cerr << "bind failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (listen(listen_sock, 3) < 0)
	{
		std::cerr << "listen" << std::endl;
		exit(EXIT_FAILURE);
	}

	epollfd = epoll_create(1);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	for (;;)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < nfds; ++n)
		{
			std::cout << "event triggerd" << std::endl;
			if (events[n].data.fd == listen_sock)
			{
				conn_sock = accept(listen_sock, (struct sockaddr *) &addr, &addrlen);
				if (conn_sock == -1)
				{
					perror("accept");
					exit(EXIT_FAILURE);
				}
				// setnonblocking(conn_sock);
				ev.events = EPOLLIN;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
				{
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				ssize_t bytes_received = read(events[n].data.fd, buffer, 1024);
				if (bytes_received == 0)
				{
					// The client has closed the connection
					std::cout << "client connection closes" << std::endl;
					close(events[n].data.fd);
				}
				else if (bytes_received == -1)
				{
					// An error occurred
					perror("read");
				}
				else
				{
					// Use the data received
					// std::cout << buffer << std::endl;
					std::string input(buffer);
					size_t pos = input.find("GET");
					if (pos != std::string::npos)
					{
						std::ifstream inputfile("index.html");
						if (!inputfile.is_open())
						{
							throw std::runtime_error("Error: could not open databasefile.");
						}
						std::cout << "size: " << httpResponse.size() << std::endl;
						readFileStream << httpResponse;
						readFileStream << inputfile.rdbuf();
						fullhttpResponse = readFileStream.str();
						readFileStream.str("");
						readFileStream.clear();
						
						send(events[n].data.fd, fullhttpResponse.c_str(), fullhttpResponse.length(), 0);
						i++;
						std::cout << "Connection number: " << i << " size readFileStream: " << readFileStream.str().size() << "size httpResponse: " << readFileStream.str().size() << std::endl;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
						close(events[n].data.fd);
						std::cout << "size: " << fullhttpResponse.size() << std::endl;
					}
					else
					{
						std::cout << "message buffer: " << buffer << std::endl;
					}
					std::memset(buffer, 0, sizeof(buffer)); // This will set all elements of the buffer to zero
				}
			}
		}
	}
}
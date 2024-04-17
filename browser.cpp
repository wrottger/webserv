/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   browser.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnebatz <dnebatz@student.42wolfsburg.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/02 22:06:18 by dnebatz           #+#    #+#             */
/*   Updated: 2024/04/17 15:13:17 by dnebatz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstring> // For memset
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()
#include <sstream>
#include <sys/epoll.h>  // für epoll_create1()
#include <cerrno>

// #define PORT 8080

int main(int argc, char **argv) {
	// Create a socket
	if (argc != 2) {
		std::cerr << "gimme port" << std::endl;
		return 1;
	}
	int port = atoi(argv[1]);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		std::cerr << "Cannot create socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	


	// Specify the server address
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);  // Server port number
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Connect to the server
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "Connection failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Send a message to the server
	std::stringstream messageStream;
	std::string message = "Hello, server!";
	if (send(sockfd, message.c_str(), message.size(), 0) < 0)
	{
		std::cerr << "Failed to send the message" << errno << std::endl;
		exit(EXIT_FAILURE);
	}
	// char buffer[1024] = {0};
	// read(sockfd, buffer, 1024);
	// std::cout << buffer << std::endl;
	while (1)
	{
		message.clear();
		std::cin >> message;
		// message << std::eof;
		if (send(sockfd, message.c_str(), message.size(), 0) < 0)
		{
			std::cerr << "Failed to send the message" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "len: " << message.size() << std::endl;
	}

	close(sockfd);

	return 0;
}
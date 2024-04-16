#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include <ctime>
#include <iostream>
#include <list>
#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include <sys/epoll.h>
#include "SocketHandling.hpp"

#define MAX_EVENTS 64
#define EPOLL_TIMEOUT 300
#define BUFFER_SIZE 8192


class EventHandler
{
	class ClientConnection;
	
	public:
		EventHandler(SocketHandling &sockets);
		~EventHandler();
		void start();

	private:
		int _epollFd;
		std::vector<int> _listeningSockets;
		std::list<EventHandler::ClientConnection *> _clientConnections;

		EventHandler();
		EventHandler(EventHandler const &other);
		EventHandler &operator =(EventHandler const &other);

		bool isListeningSocketTriggered(epoll_event events_arr[], int n);
};

class EventHandler::ClientConnection
{
	public:
		ClientConnection(int fd);
		~ClientConnection();
		int getFd();
		std::time_t getLastModified();
		void updateTime();
		bool isHeaderComplete();
		bool isBodyComplete();
		void parseBuffer(const char *buffer);

	private:
		ClientConnection();
		ClientConnection(ClientConnection const &other);
		ClientConnection &operator =(ClientConnection const &other);

		HttpRequest _requestObject;
		int _fd;
		std::time_t _lastModified;
};

#endif
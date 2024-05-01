#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>

#define MAX_EVENTS 10
#define BUF_SIZE 1000

int main() {
    int sockets[2];
    char buf[BUF_SIZE];
    int epoll_fd, nfds;
    struct epoll_event ev, events[MAX_EVENTS];
	char data[4000];
	int read_bytes = 0;

    // Fill the array with 'A'
    memset(data, 'A', sizeof(data));

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
        perror("socketpair");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) { // Child process
        close(sockets[0]);
        write(sockets[1], data, strlen(data));
        close(sockets[1]);
        exit(0);
    }

    // Parent process
    close(sockets[1]);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = sockets[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

	while (1) {
		nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(1);
		}

		for (int n = 0; n < nfds; ++n) {
			printf("Event %d\n", n);
			if (events[n].data.fd == sockets[0] && events[n].events & EPOLLIN) {
				int count = read(sockets[0], buf, BUF_SIZE);
				if (count == 0) {
					printf("Read: %d bytes\n", read_bytes);
					perror("read");
					exit(1);
				}
				read_bytes += count;
				buf[count] = '\0';
				printf("Received %d bytes: %s\n", count, buf);
			}
		}

	}
	close(sockets[0]);
    return 0;
}
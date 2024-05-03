#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define BUF_SIZE 2129920

int main() {
    int sv[2]; // the pair of socket descriptors
    char buf[BUF_SIZE]; // for incoming data
    int bytes_read;
	int total_bytes_read = 0;

    // create the socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(1);
    }

    int rcvbuf;
    int sndbuf;
    socklen_t optlen;

    optlen = sizeof(rcvbuf);
    getsockopt(sv[0], SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen);
    getsockopt(sv[0], SOL_SOCKET, SO_SNDBUF, &sndbuf, &optlen);

    printf("Receive buffer size: %d\n", rcvbuf);
    printf("Send buffer size: %d\n", sndbuf);

	if (!fork()) {  // child process
		close(sv[0]);
		char *message = "This is a message from the child process. ";
		int message_len = strlen(message);
		int array_size = message_len * 1024 * 1024;
		int bytes_written = 0;

		char *array = malloc(array_size);
		if (array == NULL) {
			fprintf(stderr, "Failed to allocate memory\n");
			exit(1);
		}
		for (int i = 0; i < array_size; i += message_len) {
			strncpy(array + i, message, message_len);
		}
		bytes_written += write(sv[1], array, array_size);  // send the data
		printf("Sent %d\n", bytes_written);
        close(sv[1]);  // child process is done
    } else {  // parent process
        close(sv[1]);
        while ((bytes_read = read(sv[0], buf, BUF_SIZE)) > 0) {  // read data in chunks of 212992 bytes
            printf("Received %d\n", bytes_read);
			usleep(100000);
			total_bytes_read += bytes_read;
        }
        if (bytes_read == -1) {
            perror("read");
            exit(1);
        }
		printf("Total bytes read: %d\n", total_bytes_read);
        close(sv[0]);
        wait(NULL);  // wait for child process to finish
    }

    return 0;
}
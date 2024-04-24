#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <wait.h>
#include <vector>


#define PARENTS_END 0
#define CHILDS_END 1

int main() {
	int sockets[2];
	// char* const argv[] = {(char*)"python3", (char*)"script.py", NULL};

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
		std::cerr << "Failed to create socket pair" << std::endl;
		return 1;
	}

	pid_t pid = fork();
	if (pid < 0) {
		std::cerr << "Failed to fork" << std::endl;
		return 1;
	}
// Child
	if (pid != 0) {
		close(sockets[PARENTS_END]);  // Close parent's end of the socket pair

		std::string executable = "/usr/bin/python3";
   		std::string firstarg = "script.py";
		std::vector<char *> argv;
		std::string big = "0";
		int i = 0;
		while (++i < 60000000) {
			big += "0";
		}
		std::cout << "size: " << big.size() << std::endl;
		if (send(sockets[CHILDS_END], big.c_str(), big.size(), 0) < 1)
			perror("send");
		// message dont wait for non blocking or using small chuncks
		// otherwise when sending to big data it will block
		// argv.push_back(const_cast<char*>(executable.c_str()));
		// argv.push_back(const_cast<char*>(firstarg.c_str()));
		// argv.push_back(NULL);
		
		// dup2(sockets[CHILDS_END], STDIN_FILENO);
		// dup2(sockets[CHILDS_END], STDOUT_FILENO);

		// execve(argv[0], argv.data(), NULL);
		// perror("execve");
		usleep(10000);
		close(sockets[CHILDS_END]);  // Close child's end of the socket pair
	} else {  // Parent process
		close(sockets[CHILDS_END]);  // Close child's end of the socket pair

		std::string string;
		int bytesReceived = 0;
		int i = 0;
		std::string realBuffer;
		while(i < 10) {
			char buffer[1000] = {0};
			bytesReceived = read(sockets[PARENTS_END], buffer, sizeof(buffer));
			if (bytesReceived <= 0) {
				break;
			}
			// std::cout << "parent received: " << buffer << std::endl;
			i++;
			realBuffer += buffer;
		}
		std::cout << realBuffer.size() << std::endl;
		close(sockets[PARENTS_END]);  // Close parent's end of the socket pair
		waitpid(pid, 0, 0);
	}
	return 0;
}

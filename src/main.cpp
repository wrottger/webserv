#include "Config.hpp"
#include "EventHandler.hpp"
#include "Logger.hpp"
#include "SocketHandling.hpp"
#include "colors.hpp"
#include "utest.h"
#include <netinet/in.h>
#include <sys/epoll.h> // für epoll_create1()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // For close()
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

UTEST_STATE();


static int decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody)
{
    if (bodyBuffer.empty()) {
        return 1;
    }
    std::stringstream bodyStream(bodyBuffer);
    for (std::string line; std::getline(bodyStream, line);)
    {
		if (!line.empty() && line.size() - 1 == '\r')
			line.erase(line.size() - 1); // Remove the trailing \r
		std::cout << "line: " << line << std::endl;
        size_t chunkSize;
        std::stringstream sizeStream(line);
		if (!(sizeStream >> std::hex >> chunkSize))
        	return -1;
		std::cout << "chunkSize: " << chunkSize << std::endl;
        if (chunkSize == 0)
            break;
        std::string chunk;
        chunk.resize(chunkSize);
        if (!bodyStream.read(&chunk[0], chunkSize))
            return -1;
        bodyStream.ignore(1);  // Ignore the trailing \r
        decodedBody += chunk;
    }
    return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
	std::string input = "4E\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n";
	std::string output;
	std::cout << decodeChunkedBody(input, output) << std::endl;
	std::cout << "|" << output << "|" << std::endl;
	// LOG_INFO("Server started");
	// // LOG_SET_LOG_LEVEL(Logging::DISABLE_LOG);
	// // LOG_DISABLE_CONSOLE_LOGGING();
	// // LOG_SET_LOG_TARGET(Logging::LOG_TO_FILE);
	// SocketHandling sockets(config->getServerBlocks());
	// EventHandler event(sockets);
	// event.start();
}

// int main()
// {
// 	/****************************************************/
// 	/*             Use with macros                      */
// 	/****************************************************/

// 	LOG_BUFFER("Raw logging without newline, timestamp or other extras\n");
// 	LOG_ERROR("This is an ERROR message with timestamp and log level to console");
// 	LOG_ALARM("This is an ALARM message with timestamp and log level to console");
// 	LOG_ALWAYS("This is an ALWAYS message with timestamp and log level to console");
// 	LOG_ENABLE_DUO_LOGGING();
// 	LOG_DISABLE_PRINT_TIME_STAMP();
// 	LOG_INFO("This is an INFO message with log level to console and file");
// 	LOG_TRACE("This is a TRACE message with log level to console and file");
// 	LOG_DISABLE_PRINT_LOG_LEVEL();
// 	LOG_DEBUG("This is a debug message without log level and timestamp to console and file");

// 	LOG_SET_LOG_LEVEL(Logging::LOG_LVL_INFO);
// 	LOG_TRACE("I will not be logged because my log level is lower than the current log level");
// 	LOG_DEBUG("I will not be logged because my log level is lower than the current log level");


// 	/****************************************************/
// 	/*             Use with Logger class                */
// 	/****************************************************/

// 	Logging::Logger &log = Logging::Logger::getInstance();
// 	log.enableAllLevels();

// 	log.disableLogging();
// 	log.error("I cannot be logged because logging is disabled");

// 	log.enableDuoLogging();
// 	log.error("I am back to be logged");

// 	log.enablePrintTimeStamp();
// 	log.error("I am logged with timestamp");

// 	log.enablePrintLogLevel();
// 	log.error("I am logged with log level and timestamp");

// 	std::ostringstream logStream;
// 	int someInt = 1337;
// 	logStream << "I am logged using a stream " << someInt << " " << 3.1415f;
// 	log.error(logStream);

// 	return 0;
// }

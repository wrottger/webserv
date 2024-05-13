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

enum ChunkState
{
	READ_SIZE,
	READ_CHUNK,
	READ_TRAILER_CR,
    READ_TRAILER_LF,
};

int decodeChunkedBody(const std::string& bodyBuffer, std::string& decodedBody)
{
    static ChunkState state = READ_SIZE;
    static bool isLastChunk = false;
    static bool isSizeRead = false;
    static std::stringstream ss;
    static unsigned int chunkSize = 0;

    for (size_t i = 0; i < bodyBuffer.size(); ++i)
    {
        switch (state)
        {
            case READ_SIZE:
            {
                if (isLastChunk)
                    return 1;
                if (bodyBuffer[i] == '\n')
                {
                    if (!(ss >> std::hex >> chunkSize))
                       throw std::runtime_error("Invalid chunk size");
                    ss.clear();
                    if (chunkSize == 0)
                    {
                        isLastChunk = true;
                        state = READ_TRAILER_CR;
                    }
                    else
                        state = READ_CHUNK;
                }
                else
                    ss << bodyBuffer[i];
                break;
            }
            case READ_CHUNK:
            {
                if (chunkSize == 0)
                    state = READ_TRAILER_CR;
                else
                {
                    decodedBody.push_back(bodyBuffer[i]);
                    chunkSize--;
                }
                break;
            }
            case READ_TRAILER_CR:
            {
                if (bodyBuffer[i] == '\r')
                    state = READ_TRAILER_LF;
                else
                    throw std::runtime_error("Invalid trailer");
                break;
            }
            case READ_TRAILER_LF:
            {
                if (bodyBuffer[i] == '\n')
                    state = READ_SIZE;
                else
                    throw std::runtime_error("Invalid trailer");
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	(void)envp;
    std::string bodyBuffer = "4\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n8\r\nWiki\r\npedia\r\n";
    std::string decodedBody;
    int ret = 0;
    for (size_t i = 0; i < bodyBuffer.size() && !ret; i += 1)
    {
        std::string chunk = bodyBuffer.substr(i, 1);
        ret = decodeChunkedBody(chunk, decodedBody);
        std::cout << "[" << i << "] Chunk: |" << chunk << "|" << std::endl;
        std::cout << "[" << i << "] Decoded body: |" << decodedBody << "|" << std::endl;
    }
    std::cout << "Finished: " << decodedBody << std::endl;
    // std::cout << "Finished: " << decodedBody << std::endl;
	// Config *config = Config::getInstance();
	// try {
	// 	config->parseConfigFile(argv[1]);
	// } catch (std::exception &e) {
	// 	std::cerr << e.what() << std::endl;
	// 	return 1;
	// }
	// if (!config->isLoaded())
	// 	return 1;
	// config->printProgressBar(1, 1);
	// std::cout << GBOLD("\nConfig file loaded successfully") << std::endl;

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

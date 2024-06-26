#include "Config.hpp"
#include "EventHandler.hpp"
#include "Logger.hpp"
#include "SocketHandling.hpp"
#include "colors.hpp"
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

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
		return 1;
	}
	Config &config = Config::getInstance();
	try {
		config.parseConfigFile(argv[1]);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if (!config.isLoaded())
		return 1;
	std::cout << GBOLD("\nConfig file loaded successfully") << std::endl;
	Logging::Logger::getInstance().startLogging();
	LOG_INFO("Server started");
	LOG_SET_LOG_LEVEL(Logging::DISABLE_LOG);
	// LOG_DISABLE_CONSOLE_LOGGING();
	// LOG_SET_LOG_TARGET(Logging::LOG_TO_FILE);
	EventHandler::getInstance().start();
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
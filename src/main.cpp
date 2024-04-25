#include "Logger.hpp"

int main()
{
	/****************************************************/
	/*             Use with macros                      */
	/****************************************************/

	LOG_BUFFER("Raw logging without newline, timestamp or other extras\n");
	LOG_ERROR("This is an ERROR message with timestamp and log level to console");
	LOG_ALARM("This is an ALARM message with timestamp and log level to console");
	LOG_ALWAYS("This is an ALWAYS message with timestamp and log level to console");
	LOG_ENABLE_DUO_LOGGING();
	LOG_DISABLE_PRINT_TIME_STAMP();
	LOG_INFO("This is an INFO message with log level to console and file");
	LOG_TRACE("This is a TRACE message with log level to console and file");
	LOG_DISABLE_PRINT_LOG_LEVEL();
	LOG_DEBUG("This is a debug message without log level and timestamp to console and file");

	LOG_SET_LOG_LEVEL(Logging::LOG_LVL_INFO);
	LOG_TRACE("I will not be logged because my log level is lower than the current log level");
	LOG_DEBUG("I will not be logged because my log level is lower than the current log level");


	/****************************************************/
	/*             Use with Logger class                */
	/****************************************************/

	Logging::Logger &log = Logging::Logger::getInstance();
	log.enableAllLevels();

	log.disableLogging();
	log.error("I cannot be logged because logging is disabled");

	log.enableDuoLogging();
	log.error("I am back to be logged");

	log.enablePrintTimeStamp();
	log.error("I am logged with timestamp");

	log.enablePrintLogLevel();
	log.error("I am logged with log level and timestamp");

	std::ostringstream logStream;
	int someInt = 1337;
	logStream << "I am logged using a stream " << someInt << " " << 3.1415f;
	log.error(logStream);
	LOG_ERROR(logStream);

	LOG_ERROR_WITH_TAG("I am logged with a tag", "EventHandler");
	LOG_DISABLE_PRINT_TIME_STAMP();
	LOG_DISABLE_PRINT_LOG_LEVEL();
	LOG_ALARM_WITH_TAG("I am logged with a tag", "EventHandler");

	return 0;
}
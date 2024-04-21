#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <sys/time.h>

namespace Logging {
	// Log file name
	static const std::string LOG_SAVE_FILE = "log.txt";

	// Direct interface for logging using a MACRO
	#define LOG_ERROR(text)		Logging::Logger::getInstance().error(text)	// log error message
	#define LOG_ALARM(text)		Logging::Logger::getInstance().alarm(text)	// log alarm message
	#define LOG_ALWAYS(text)	Logging::Logger::getInstance().always(text)	// log always message
	#define LOG_BUFFER(text)	Logging::Logger::getInstance().buffer(text)	// log RAW buffer message
	#define LOG_INFO(text)		Logging::Logger::getInstance().info(text)	// log info message
	#define LOG_TRACE(text)		Logging::Logger::getInstance().trace(text)	// log trace message
	#define LOG_DEBUG(text)		Logging::Logger::getInstance().debug(text)	// log debug message

	// Interfaces to control log levels and log types
	#define LOG_SET_LOG_LEVEL(level)	Logging::Logger::getInstance().setLogLevel(level)		// Set log level
	#define LOG_SET_LOG_TARGET(type)		Logging::Logger::getInstance().setLogTarget(type)	// Set log target

	// Interfaces to enable and disable logging
	#define LOG_ENABLE_ALL_LEVELS()		Logging::Logger::getInstance().enableAllLevels()	// Enable all log levels
	#define LOG_DISABLE_ALL_LEVELS()	Logging::Logger::getInstance().disableAllLevels()	// Disable all log levels, except error and alarm

	// Interfaces to enable and disable console and file logging
	#define LOG_ENABLE_CONSOLE_LOGGING()	Logging::Logger::getInstance().enableConsoleLogging()	// Enable console logging
	#define LOG_ENABLE_FILE_LOGGING()		Logging::Logger::getInstance().enableFileLogging()		// Enable file logging
	#define LOG_DISABLE_CONSOLE_LOGGING()	Logging::Logger::getInstance().disableConsoleLogging()	// Disable console logging
	#define LOG_DISABLE_FILE_LOGGING()		Logging::Logger::getInstance().disableFileLogging()		// Disable file logging
	#define LOG_ENABLE_DUO_LOGGING()		Logging::Logger::getInstance().enableDuoLogging()		// Enable both console and file logging

	// Interfaces to enable and disable time stamp and log level in log
	#define LOG_ENABLE_PRINT_TIME_STAMP()	Logging::Logger::getInstance().enablePrintTimeStamp()	// Enable time stamp in log
	#define LOG_DISABLE_PRINT_TIME_STAMP()	Logging::Logger::getInstance().disablePrintTimeStamp()	// Disable time stamp in log
	#define LOG_ENABLE_PRINT_LOG_LEVEL()	Logging::Logger::getInstance().enablePrintLogLevel()	// Enable log level in log
	#define LOG_DISABLE_PRINT_LOG_LEVEL()	Logging::Logger::getInstance().disablePrintLogLevel()	// Disable log level in log

	typedef enum LOG_LEVEL {
		DISABLE_LOG,
		LOG_LVL_INFO,
		LOG_LVL_BUFFER,
		LOG_LVL_TRACE,
		LOG_LVL_DEBUG,
		ENABLE_LOG
	} LogLevel;

	typedef enum LOG_TARGET {
		LOG_NONE = 0,
		LOG_TO_CONSOLE = 1,
		LOG_TO_FILE = 2,
		LOG_TO_BOTH = 3
	} LogTarget;

	class Logger {
	public:

		static Logger& getInstance(); // Return the instance of the Logger

		void error(const char* text);			// log error message
		void error(std::string& text);			// log error message
		void error(std::ostringstream& stream);	// log error message

		void alarm(const char* text);			// log alarm message
		void alarm(std::string& text);			// log alarm message
		void alarm(std::ostringstream& stream); // log alarm message

		void always(const char* text);			// log always message
		void always(std::string& text);			// log always message
		void always(std::ostringstream& stream);// log always message

		void buffer(const char* text);			// log RAW buffer message
		void buffer(std::string& text);			// log RAW buffer message
		void buffer(std::ostringstream& stream);// log RAW buffer message

		void info(const char* text);			// log info message
		void info(std::string& text);			// log info message
		void info(std::ostringstream& stream);	// log info message

		void trace(const char* text);			// log trace message
		void trace(std::string& text);			// log trace message
		void trace(std::ostringstream& stream);	// log trace message

		void debug(const char* text);			// log debug message
		void debug(std::string& text);			// log debug message
		void debug(std::ostringstream& stream);	// log debug message

		// Error and Alarm log are always enabled
		// Hence, there is no interfce to control error and alarm logs

		// Interfaces to control log levels
		void setLogLevel(LogLevel logLevel); // Set log level
		void enableAllLevels();					// Enable all log levels
		void disableAllLevels();				// Disable all log levels, except error and alarm

		void setLogTarget(LogTarget logTarget); // Set log type

		// Interfaces to enable and disable logging
		void enableConsoleLogging();	// Enable console logging
		void enableFileLogging();		// Enable file logging
		void disableConsoleLogging();	// Disable console logging
		void disableFileLogging();		// Disable file logging
		void enableDuoLogging();		// Enable both console and file logging
		void disableLogging();			// Disable both console and file logging

		void enablePrintTimeStamp();	// Enable time stamp in log
		void disablePrintTimeStamp();	// Disable time stamp in log
		void enablePrintLogLevel();		// Enable log level in log
		void disablePrintLogLevel();	// Disable log level in log

	protected:
		Logger();
		~Logger();


	private:
		static Logger* _instance;
		std::ofstream _logFile;
		LogLevel _logLevel;
		LogTarget _logTarget;
		bool _timeStampInLog;
		bool _logLevelInLog;

	private:
		std::string getCurrentTime();
		void logMessage(const char* level, const char* text);
		void createLogMessage(std::string &buffer, const char *logLevel, const char *text);
		void writeLog(const std::string &data);
		void startLogging();
		std::string insertMetaInformations(const char *logLevel);

		Logger(const Logger& other);				// Copy constructor
		Logger& operator=(const Logger& other);		// Assignment operator
	};
}

#endif // LOGGING_HPP

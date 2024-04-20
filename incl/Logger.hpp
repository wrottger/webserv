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
	#define LOG_ERROR(text)		Logging::Logger::getInstance().error(text) // Error log
	#define LOG_ALARM(text)		Logging::Logger::getInstance().alarm(text) // Alarm log
	#define LOG_ALWAYS(text)	Logging::Logger::getInstance().always(text) // Always log
	#define LOG_BUFFER(text)	Logging::Logger::getInstance().buffer(text) // Buffer log
	#define LOG_INFO(text)		Logging::Logger::getInstance().info(text) // Info log
	#define LOG_TRACE(text)		Logging::Logger::getInstance().trace(text) // Trace log
	#define LOG_DEBUG(text)		Logging::Logger::getInstance().debug(text) // Debug log

	// Interfaces to control log levels and log types
	#define LOG_SET_LOG_LEVEL(level) Logging::Logger::getInstance().updateLogLevel(level) // Set log level
	#define LOG_SET_LOG_TYPE(type) Logging::Logger::getInstance().updateLogType(type) // Set log type

	// Interfaces to enable and disable logging
	#define LOG_ENABLE_LOGGING()	Logging::Logger::getInstance().enableLogs() // Enable all log levels
	#define LOG_DISABLE_LOGGING()	Logging::Logger::getInstance().disableLogs() // Disable all log levels, except error and alarm

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

	typedef enum LOG_TYPE {
		LOG_NONE = 0,
		LOG_TO_CONSOLE = 1,
		LOG_TO_FILE = 2,
		LOG_TO_BOTH = 3
	} LogType;

	class Logger {
	public:

		static Logger& getInstance() throw();
		void deleteInstance() throw();

		// Interface for Error Log 
		void error(const char* text) throw();
		void error(std::string& text) throw();
		void error(std::ostringstream& stream) throw();

		// Interface for Alarm Log 
		void alarm(const char* text) throw();
		void alarm(std::string& text) throw();
		void alarm(std::ostringstream& stream) throw();

		// Interface for Always Log 
		void always(const char* text) throw();
		void always(std::string& text) throw();
		void always(std::ostringstream& stream) throw();

		// Interface for Buffer Log 
		void buffer(const char* text) throw();
		void buffer(std::string& text) throw();
		void buffer(std::ostringstream& stream) throw();

		// Interface for Info Log 
		void info(const char* text) throw();
		void info(std::string& text) throw();
		void info(std::ostringstream& stream) throw();

		// Interface for Trace log 
		void trace(const char* text) throw();
		void trace(std::string& text) throw();
		void trace(std::ostringstream& stream) throw();

		// Interface for Debug log 
		void debug(const char* text) throw();
		void debug(std::string& text) throw();
		void debug(std::ostringstream& stream) throw();

		// Error and Alarm log must be always enable
		// Hence, there is no interfce to control error and alarm logs

		// Interfaces to control log levels
		void updateLogLevel(LogLevel logLevel);
		void enableLogs();  // Enable all log levels
		void disableLogs(); // Disable all log levels, except error and alarm

		// Interfaces to control log Types
		void updateLogType(LogType logType);

		// Interfaces to enable and disable logging
		void enableConsoleLogging();
		void enableFileLogging();
		void disableConsoleLogging();
		void disableFileLogging();
		void enableDuoLogging(); // Enable both console and file logging
		void disableLogging(); // Disable both console and file logging

		// Interfaces to enable and disable time stamp and log level in log
		void enablePrintTimeStamp();
		void disablePrintTimeStamp();
		void enablePrintLogLevel();
		void disablePrintLogLevel();

	protected:
		Logger();
		~Logger();

		std::string getCurrentTime();

	private:
		static Logger* _instance;
		std::ofstream _logFile;
		LogLevel _logLevel;
		LogType _logType;
		bool _timeStampInLog;
		bool _logLevelInLog;

	private:
		void writeLog(std::string &data);
		void startLogging();
		std::string insertMetaInformations(const char *logLevel);

		Logger(const Logger& other); // Copy constructor
		Logger& operator=(const Logger& other); // Assignment operator
	};
}

#endif // LOGGING_HPP

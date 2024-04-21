#include "Logger.hpp"

Logging::Logger *Logging::Logger::_instance = NULL;

Logging::Logger::Logger() {
	_logFile.open(LOG_SAVE_FILE.c_str(), std::ios::out | std::ios::app);
	if (_logFile.fail()) {
		std::cerr << "Failed to open log file: " << LOG_SAVE_FILE << std::endl;
	}
	_logLevel = LOG_LVL_DEBUG;	// Default log level
	_logType = LOG_TO_CONSOLE;	// Default log type
	_timeStampInLog = true;		// Default timestamp in log
	_logLevelInLog = true;		// Default log level in log
	startLogging();
}

Logging::Logger::~Logger() {
	_logFile.close();
}

// Singleton instance
Logging::Logger &Logging::Logger::getInstance() {
	static Logger _instance;
	return _instance;
}

void Logging::Logger::startLogging() {
	buffer("************************************************\n");
	buffer("*   Logging started: ");
	buffer(getCurrentTime().c_str());
	buffer("   *\n");
	buffer("************************************************\n");
}

std::string Logging::Logger::getCurrentTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	std::time_t rawtime = tv.tv_sec;
	std::tm *timeinfo = std::localtime(&rawtime);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

	std::ostringstream oss;
	oss << buffer << "." << std::setfill('0') << std::setw(3) << tv.tv_usec / 1000;

	return oss.str();
}

void Logging::Logger::logMessage(const char* level, const char* text) {
	std::string logline;
	createLogMessage(logline, level, text);
	writeLog(logline);
}

void Logging::Logger::createLogMessage(std::string &buffer, const char *logLevel, const char *text) {
	buffer.append(insertMetaInformations(logLevel));
	buffer.append(text);
}

void Logging::Logger::writeLog(const std::string &data) {
	// Write log into file
	if (_logFile.rdstate() == std::ios_base::goodbit) {
		if (LOG_TO_FILE == static_cast<LogType>(_logType & LOG_TO_FILE)) {
			_logFile << data << std::endl;
		}
	}
	// Write log into console
	if (LOG_TO_CONSOLE == static_cast<LogType>(_logType & LOG_TO_CONSOLE)) {
		std::cout << data << std::endl;
	}
}

std::string Logging::Logger::insertMetaInformations(const char *logLevel) {
	std::string data;

	if (_timeStampInLog) {
		data.append(getCurrentTime()).append(" ");
	}

	if (_logLevelInLog) {
		data.append("[").append(logLevel).append("]").append(": ");
	}

	return data;
}

// Interface for Error Log
void Logging::Logger::error(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	// ERROR must be captured
	logMessage("ERROR", text);
}

void Logging::Logger::error(std::string &text) {
	error(text.data());
}

void Logging::Logger::error(std::ostringstream &stream) {
	std::string text = stream.str();
	error(text.data());
}

// Interface for Alarm Log
void Logging::Logger::alarm(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	// ALARM must be captured
	logMessage("ALARM", text);
}

void Logging::Logger::alarm(std::string &text) {
	alarm(text.data());
}

void Logging::Logger::alarm(std::ostringstream &stream) {
	std::string text = stream.str();
	alarm(text.data());
}

// Interface for Always Log
void Logging::Logger::always(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	// ALWAYS must be captured
	logMessage("ALWAYS", text);
}

void Logging::Logger::always(std::string &text) {
	always(text.data());
}

void Logging::Logger::always(std::ostringstream &stream) {
	std::string text = stream.str();
	always(text.data());
}

// Interface for Buffer Log
void Logging::Logger::buffer(const char *text) {
	// Buffer is the special case. So don't add log level
	// and timestamp in the buffer message. Just log the raw bytes.
	if (_logType == LOG_NONE) {
		return;
	}
	if (LOG_TO_FILE == static_cast<LogType>(_logType & LOG_TO_FILE)) {
		_logFile << text;
	}
	// Write log into console
	if (LOG_TO_CONSOLE == static_cast<LogType>(_logType & LOG_TO_CONSOLE)) {
		std::cout << text;
	}
}

void Logging::Logger::buffer(std::string &text) {
	buffer(text.data());
}

void Logging::Logger::buffer(std::ostringstream &stream) {
	std::string text = stream.str();
	buffer(text.data());
}

// Interface for Info Log
void Logging::Logger::info(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_INFO) {
		logMessage("INFO", text);
	}
}

void Logging::Logger::info(std::string &text) {
	info(text.data());
}

void Logging::Logger::info(std::ostringstream &stream) {
	std::string text = stream.str();
	info(text.data());
}

// Interface for Trace Log
void Logging::Logger::trace(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_TRACE) {
		logMessage("TRACE", text);
	}
}

void Logging::Logger::trace(std::string &text) {
	trace(text.data());
}

void Logging::Logger::trace(std::ostringstream &stream) {
	std::string text = stream.str();
	trace(text.data());
}

// Interface for Debug Log
void Logging::Logger::debug(const char *text) {
	if (_logType == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_DEBUG) {
		logMessage("DEBUG", text);
	}
}

void Logging::Logger::debug(std::string &text) {
	debug(text.data());
}

void Logging::Logger::debug(std::ostringstream &stream) {
	std::string text = stream.str();
	debug(text.data());
}

// Interfaces to control log levels
void Logging::Logger::updateLogLevel(LogLevel logLevel) {
	_logLevel = logLevel;
}

// Enable all log levels
void Logging::Logger::enableLogs() {
	_logLevel = ENABLE_LOG;
}

// Disable all log levels, except error and alarm
void Logging::Logger::disableLogs() {
	_logLevel = DISABLE_LOG;
}

// Interfaces to control log Types
void Logging::Logger::updateLogType(LogType logType) {
	_logType = static_cast<LogType>(_logType | logType);
}

void Logging::Logger::enableFileLogging() {
	_logType = static_cast<LogType>(_logType | LOG_TO_FILE);
}

void Logging::Logger::enableConsoleLogging() {
	_logType = static_cast<LogType>(_logType | LOG_TO_CONSOLE);
}

void Logging::Logger::disableFileLogging() {
	_logType = static_cast<LogType>(_logType & ~LOG_TO_FILE);
}

void Logging::Logger::disableConsoleLogging() {
	_logType = static_cast<LogType>(_logType & ~LOG_TO_CONSOLE);
}

void Logging::Logger::enableDuoLogging() {
	_logType = LOG_TO_BOTH;
}

void Logging::Logger::disableLogging() {
	_logType = LOG_NONE;
}

void Logging::Logger::enablePrintTimeStamp() {
	_timeStampInLog = true;
}

void Logging::Logger::disablePrintTimeStamp() {
	_timeStampInLog = false;
}

void Logging::Logger::enablePrintLogLevel() {
	_logLevelInLog = true;
}

void Logging::Logger::disablePrintLogLevel() {
	_logLevelInLog = false;
}

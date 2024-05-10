#include "Logger.hpp"

Logging::Logger Logging::Logger::_instance;

Logging::Logger::Logger() {
	_logFile.open(LOG_SAVE_FILE.c_str(), std::ios::out | std::ios::app);
	if (_logFile.fail()) {
		std::cerr << "Failed to open log file: " << LOG_SAVE_FILE << std::endl;
	}
	_logLevel = LOG_LVL_DEBUG;		// Default log level
	_logTarget = LOG_TO_CONSOLE;	// Default log type
	_timeStampInLog = true;			// Default timestamp in log
	_logLevelInLog = true;			// Default log level in log
}

Logging::Logger::~Logger() {
	_logFile.close();
}

// Singleton instance
Logging::Logger &Logging::Logger::getInstance() {
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

// Creates and writes log message
void Logging::Logger::logMessage(const char* level, const char* text) {
	std::string logline;
	createLogMessage(logline, level, text);
	writeLog(logline);
}

// Creates and writes log message with tag
void Logging::Logger::logMessage(const char* level, const char* text, const char* tag) {
	std::string logline;
	createLogMessage(logline, level, text, tag);
	writeLog(logline);
}

// Creates log message
void Logging::Logger::createLogMessage(std::string &buffer, const char *logLevel, const char *text) {
	buffer.append(insertMetaInformations(logLevel));
	buffer.append(text);
}

void Logging::Logger::createLogMessage(std::string &buffer, const char *logLevel, const char *text, const char *tag) {
	buffer.append(insertMetaInformations(logLevel, tag));
	buffer.append(text);
}

// Writes log message
void Logging::Logger::writeLog(const std::string &data) {
	// Write log into file
	if (_logFile.rdstate() == std::ios_base::goodbit) {
		if (LOG_TO_FILE == static_cast<LogTarget>(_logTarget & LOG_TO_FILE)) {
			_logFile << data << std::endl;
		}
	}
	// Write log into console
	if (LOG_TO_CONSOLE == static_cast<LogTarget>(_logTarget & LOG_TO_CONSOLE)) {
		std::cerr << data << std::endl;
	}
}

// Inserts timestamp and log level in log
std::string Logging::Logger::insertMetaInformations(const char *logLevel) {
	std::string data;

	if (_timeStampInLog) {
		data.append(getCurrentTime()).append(" ");
	}

	if (_logLevelInLog) {
		data.append("[").append(logLevel).append("] ");
	}

	return data;
}

// Inserts timestamp, log level and tag in log
std::string Logging::Logger::insertMetaInformations(const char *logLevel, const char *tag) {
	std::string data;

	if (_timeStampInLog) {
		data.append(getCurrentTime()).append(" ");
	}

	if (tag != NULL) {
		data.append("[").append(tag).append("] ");
	}

	if (_logLevelInLog) {
		data.append("[").append(logLevel).append("] ");
	}
	return data;
}

// log error message
void Logging::Logger::error(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ERROR must be captured
	logMessage("ERROR", text);
}

// log error message
void Logging::Logger::error(std::string &text) {
	error(text.data());
}

// log error message
void Logging::Logger::error(std::ostringstream &stream) {
	std::string text = stream.str();
	error(text.data());
}

// log error message with tag
void Logging::Logger::error(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ERROR must be captured
	logMessage("ERROR", text, tag);
}

// log error message with tag
void Logging::Logger::error(std::string &text, const char *tag) {
	error(text.data(), tag);
}

// log error message with tag
void Logging::Logger::error(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	error(text.data(), tag);
}

// log alarm message
void Logging::Logger::alarm(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ALARM must be captured
	logMessage("ALARM", text);
}

// log alarm message
void Logging::Logger::alarm(std::string &text) {
	alarm(text.data());
}

// log alarm message
void Logging::Logger::alarm(std::ostringstream &stream) {
	std::string text = stream.str();
	alarm(text.data());
}

// log alarm message with tag
void Logging::Logger::alarm(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ALARM must be captured
	logMessage("ALARM", text, tag);
}

// log alarm message with tag
void Logging::Logger::alarm(std::string &text, const char *tag) {
	alarm(text.data(), tag);
}

// log alarm message with tag
void Logging::Logger::alarm(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	alarm(text.data(), tag);
}

// log always message
void Logging::Logger::always(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ALWAYS must be captured
	logMessage("ALWAYS", text);
}

// log always message
void Logging::Logger::always(std::string &text) {
	always(text.data());
}

// log always message
void Logging::Logger::always(std::ostringstream &stream) {
	std::string text = stream.str();
	always(text.data());
}

// log always message with tag
void Logging::Logger::always(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	// ALWAYS must be captured
	logMessage("ALWAYS", text, tag);
}

// log always message with tag
void Logging::Logger::always(std::string &text, const char *tag) {
	always(text.data(), tag);
}

// log always message with tag
void Logging::Logger::always(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	always(text.data(), tag);
}

// log RAW buffer message
void Logging::Logger::buffer(const char *text) {
	// Buffer is the special case. So don't add log level
	// and timestamp in the buffer message. Just log the raw bytes.
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (LOG_TO_FILE == static_cast<LogTarget>(_logTarget & LOG_TO_FILE)) {
		_logFile << text;
	}
	// Write log into console
	if (LOG_TO_CONSOLE == static_cast<LogTarget>(_logTarget & LOG_TO_CONSOLE)) {
		std::cerr << text;
	}
}

// log RAW buffer message
void Logging::Logger::buffer(std::string &text) {
	buffer(text.data());
}

// log RAW buffer message
void Logging::Logger::buffer(std::ostringstream &stream) {
	std::string text = stream.str();
	buffer(text.data());
}

// log info message
void Logging::Logger::info(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_INFO) {
		logMessage("INFO", text);
	}
}

// log info message
void Logging::Logger::info(std::string &text) {
	info(text.data());
}

// log info message
void Logging::Logger::info(std::ostringstream &stream) {
	std::string text = stream.str();
	info(text.data());
}

// log info message with tag
void Logging::Logger::info(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_INFO) {
		logMessage("INFO", text, tag);
	}
}

// log info message with tag
void Logging::Logger::info(std::string &text, const char *tag) {
	info(text.data(), tag);
}

// log info message with tag
void Logging::Logger::info(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	info(text.data(), tag);
}

// log trace message
void Logging::Logger::trace(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_TRACE) {
		logMessage("TRACE", text);
	}
}

// log trace message
void Logging::Logger::trace(std::string &text) {
	trace(text.data());
}

// log trace message
void Logging::Logger::trace(std::ostringstream &stream) {
	std::string text = stream.str();
	trace(text.data());
}

// log trace message with tag
void Logging::Logger::trace(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_TRACE) {
		logMessage("TRACE", text, tag);
	}
}

// log trace message with tag
void Logging::Logger::trace(std::string &text, const char *tag) {
	trace(text.data(), tag);
}

// log debug message with tag
void Logging::Logger::trace(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	trace(text.data(), tag);
}

// log debug message
void Logging::Logger::debug(const char *text) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_DEBUG) {
		logMessage("DEBUG", text);
	}
}

// log debug message
void Logging::Logger::debug(std::string &text) {
	debug(text.data());
}

// log debug message
void Logging::Logger::debug(std::ostringstream &stream) {
	std::string text = stream.str();
	debug(text.data());
}

// log debug message with tag
void Logging::Logger::debug(const char *text, const char *tag) {
	if (_logTarget == LOG_NONE) {
		return;
	}
	if (_logLevel >= LOG_LVL_DEBUG) {
		logMessage("DEBUG", text, tag);
	}
}

// log debug message with tag
void Logging::Logger::debug(std::string &text, const char *tag) {
	debug(text.data(), tag);
}

// log debug message with tag
void Logging::Logger::debug(std::ostringstream &stream, const char *tag) {
	std::string text = stream.str();
	debug(text.data(), tag);
}

// Set log level
void Logging::Logger::setLogLevel(LogLevel logLevel) {
	_logLevel = logLevel;
}

// Enable all log levels
void Logging::Logger::enableAllLevels() {
	_logLevel = ENABLE_LOG;
}

// Disable all log levels, except error and alarm
void Logging::Logger::disableAllLevels() {
	_logLevel = DISABLE_LOG;
}

// Interfaces to control log Types
void Logging::Logger::setLogTarget(LogTarget logType) {
	_logTarget = static_cast<LogTarget>(_logTarget | logType);
}

// Enable file logging
void Logging::Logger::enableFileLogging() {
	_logTarget = static_cast<LogTarget>(_logTarget | LOG_TO_FILE);
}

// Enable console logging
void Logging::Logger::enableConsoleLogging() {
	_logTarget = static_cast<LogTarget>(_logTarget | LOG_TO_CONSOLE);
}

// Disable file logging
void Logging::Logger::disableFileLogging() {
	_logTarget = static_cast<LogTarget>(_logTarget & ~LOG_TO_FILE);
}

// Disable console logging
void Logging::Logger::disableConsoleLogging() {
	_logTarget = static_cast<LogTarget>(_logTarget & ~LOG_TO_CONSOLE);
}

// Enable both file and console logging
void Logging::Logger::enableDuoLogging() {
	_logTarget = LOG_TO_BOTH;
}

// Disable both file and console logging
void Logging::Logger::disableLogging() {
	_logTarget = LOG_NONE;
}

// Show timestamp in log
void Logging::Logger::enablePrintTimeStamp() {
	_timeStampInLog = true;
}

// Hide timestamp in log
void Logging::Logger::disablePrintTimeStamp() {
	_timeStampInLog = false;
}

// Show log level in log
void Logging::Logger::enablePrintLogLevel() {
	_logLevelInLog = true;
}

// Hide log level in log
void Logging::Logger::disablePrintLogLevel() {
	_logLevelInLog = false;
}

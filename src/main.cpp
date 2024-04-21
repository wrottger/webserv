#include "Logger.hpp"

int main()
{
	LOG_ENABLE_FILE_LOGGING();
	LOG_ERROR("This is an error message");
	LOG_ALARM("This is an alarm message");
	LOG_ALWAYS("This is an always message");
	LOG_BUFFER("This is a buffer message\n");
	LOG_INFO("This is an info message");
	LOG_TRACE("This is a trace message");
	LOG_DEBUG("This is a debug message");

	LOG_SET_LOG_LEVEL(Logging::LOG_LVL_INFO);

	LOG_DISABLE_FILE_LOGGING();
	LOG_ERROR("This is an error message");
	LOG_ALARM("This is an alarm message");
	LOG_ALWAYS("This is an always message");
	LOG_BUFFER("This is a buffer message\n");
	LOG_INFO("This is an info message");
	LOG_TRACE("This is a trace message");
	LOG_DEBUG("This is a debug message");

	LOG_ENABLE_LOGGING();
	LOG_DISABLE_LOGGING();

	LOG_ENABLE_CONSOLE_LOGGING();
	LOG_DISABLE_CONSOLE_LOGGING();
	LOG_ENABLE_FILE_LOGGING();
	LOG_DISABLE_FILE_LOGGING();
	LOG_ENABLE_DUO_LOGGING();

	LOG_ENABLE_PRINT_TIME_STAMP();
	LOG_DISABLE_PRINT_TIME_STAMP();
	LOG_ENABLE_PRINT_LOG_LEVEL();
	LOG_DISABLE_PRINT_LOG_LEVEL();

	return 0;
}
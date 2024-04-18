#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    Logger(const std::string& filename) : logFile(filename, std::ios::app) {}

    void log(Level level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile << "[" << getLevelString(level) << "] " << message << std::endl;
    }

private:
    std::string getLevelString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARN: return "WARN";
            case Level::ERROR: return "ERROR";
            case Level::FATAL: return "FATAL";
            default: return "";
        }
    }

    std::ofstream logFile;
    std::mutex logMutex;
};
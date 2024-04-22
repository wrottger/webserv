#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <map>
#include "States.hpp"
#include "HttpMessage.hpp"
#include "HttpError.hpp"

struct StateHandler;

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();
    size_t parseBuffer(const char *requestLine);
    const std::string &getMethod() const;
    const std::string &getPath() const;
    const std::string &getQuery() const;
    const std::string &getHeader(const std::string &name) const;
    const std::string &getBody() const;

    HttpError getError() const;

    bool isComplete() const;
    bool isHeaderComplete() const;

private:
    static const size_t MAX_REQUEST_SIZE = 8192;
    static const size_t MAX_HEADER_SIZE = 4096;
    static const size_t MAX_URI_SIZE = 4096;

    bool headerComplete;
    bool complete;

    StateHandler *state;
    HttpMessage message;
    size_t request_size;

    HttpError parseError;

    std::string method; // GET, POST etc.
    std::string host; // localhost, google.com etc.
    std::string path; // /, /index.html etc.
    std::string query; // ?key=value etc.
    std::string fragment; // #fragment etc.
    std::map<std::string, std::string> headers;

    std::string body;

    HttpRequest(const HttpRequest &other);
    HttpRequest &operator=(const HttpRequest &other);

        
};

#endif
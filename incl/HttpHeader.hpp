#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <map>
#include "HttpError.hpp"

class HttpHeader {
public:
    HttpHeader();
    ~HttpHeader();
    size_t parseBuffer(const char *requestLine);
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getMethod() const;
    const std::string &getPath() const;
    const std::string &getFileExtension() const;
    const std::string &getQuery() const;
    const std::string getHeader(const std::string &name) const;
    const std::string &getBody() const;

    HttpError getError() const;
    bool isError() const;

    bool isComplete() const;

private:
    struct HttpMessage {
        std::string method;
        std::string path;
        std::string query;
        std::string fragment;
        std::string fieldName;
        std::string fieldValue;
        std::map<std::string, std::string> headers;
        std::string body;
    };
    bool complete;

    HttpMessage message;
    size_t request_size;

    HttpError parseError;

    HttpHeader(const HttpHeader &other);
    std::string percentDecode(std::string &str);
    HttpHeader &operator=(const HttpHeader &other);

    struct StateHandler;
    typedef void (*StateHandlerFunc)(char c, HttpMessage& message, StateHandler& nextState);
    struct StateHandler {
        StateHandlerFunc func;
    };
    StateHandler *state;

    struct States
    {
        static bool isToken(char c);
        static bool isPchar(char c);

        static void method(char c, HttpMessage& message, StateHandler& nextState);

        // URI
        static void targetStart(char c, HttpMessage& message, StateHandler& nextState);
        static void scheme(char c, HttpMessage& message, StateHandler& nextState);
        static void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState);
        static void authority(char c, HttpMessage& message, StateHandler& nextState);
        static void port(char c, HttpMessage& message, StateHandler& nextState);
        static void path(char c, HttpMessage& message, StateHandler& nextState);
        static void query(char c, HttpMessage& message, StateHandler& nextState);
        static void fragment(char c, HttpMessage& message, StateHandler& nextState);

        // HTTP version
        static void httpVersion(char c, HttpMessage& message, StateHandler& nextState);
        static void httpSlash(char c, HttpMessage& message, StateHandler& nextState);
        static void ht(char c, HttpMessage& message, StateHandler& nextState);
        static void htt(char c, HttpMessage& message, StateHandler& nextState);
        static void http(char c, HttpMessage& message, StateHandler& nextState);
        static void majorDigit(char c, HttpMessage& message, StateHandler& nextState);
        static void minorDigit(char c, HttpMessage& message, StateHandler& nextState);
        static void dot(char c, HttpMessage& message, StateHandler& nextState);

        // Headers
        static void NL(char c, HttpMessage& message, StateHandler& nextState);
        static void CR(char c, HttpMessage& message, StateHandler& nextState);
        static void fieldName(char c, HttpMessage& message, StateHandler& nextState);
        static void OWS(char c, HttpMessage& message, StateHandler& nextState);
        static void SP(char c, HttpMessage& message, StateHandler& nextState);
        static void fieldValue(char c, HttpMessage& message, StateHandler& nextState);
        static void headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState);
        static void headerFinished(char c, HttpMessage& message, StateHandler& nextState);
    }; // struct States
};

#endif
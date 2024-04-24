#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <map>
#include "States.hpp"
#include "HttpMessage.hpp"
#include "HttpError.hpp"

struct StateHandler;

class HttpHeader {
public:
    HttpHeader();
    ~HttpHeader();
    size_t parseBuffer(const char *requestLine);
    const std::string &getMethod() const;
    const std::string &getPath() const;
    const std::string &getFileExtension() const;
    const std::string &getQuery() const;
    const std::string &getHeader(const std::string &name) const;
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

    struct StateHandler;

typedef void (*StateHandlerFunc)(char c, HttpMessage& message, StateHandler& nextState);

struct StateHandler {
    StateHandlerFunc func;
};

struct States
{

    static bool isToken(char c) {
        const std::string delimiters = std::string("\"(),/:;<=>?@[\\]{}");
        return (c > 32 && c < 127) && delimiters.find(c) == std::string::npos;
    }

    static bool isPchar(char c) {
        const static std::string subDelis = std::string("!$&'()*+,;=");
        return isalnum(c) || subDelis.find(c) != std::string::npos || c == ':' || c == '@' || c == '%';
    }

    // static void method(char c, HttpMessage& message, StateHandler& nextState);

    // // URI
    // static void targetStart(char c, HttpMessage& message, StateHandler& nextState);
    // static void scheme(char c, HttpMessage& message, StateHandler& nextState);
    // static void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState);
    // static void authority(char c, HttpMessage& message, StateHandler& nextState);
    // static void port(char c, HttpMessage& message, StateHandler& nextState);
    // static void path(char c, HttpMessage& message, StateHandler& nextState);
    // static void query(char c, HttpMessage& message, StateHandler& nextState);
    // static void fragment(char c, HttpMessage& message, StateHandler& nextState);

    // // HTTP version
    // static void httpVersion(char c, HttpMessage& message, StateHandler& nextState);
    // static void httpSlash(char c, HttpMessage& message, StateHandler& nextState);
    // static void ht(char c, HttpMessage& message, StateHandler& nextState);
    // static void htt(char c, HttpMessage& message, StateHandler& nextState);
    // static void http(char c, HttpMessage& message, StateHandler& nextState);
    // static void majorDigit(char c, HttpMessage& message, StateHandler& nextState);
    // static void minorDigit(char c, HttpMessage& message, StateHandler& nextState);
    // static void dot(char c, HttpMessage& message, StateHandler& nextState);

    // // Headers
    // static void NL(char c, HttpMessage& message, StateHandler& nextState);
    // static void CR(char c, HttpMessage& message, StateHandler& nextState);
    // static void fieldName(char c, HttpMessage& message, StateHandler& nextState);
    // static void OWS(char c, HttpMessage& message, StateHandler& nextState);
    // static void SP(char c, HttpMessage& message, StateHandler& nextState);
    // static void fieldValue(char c, HttpMessage& message, StateHandler& nextState);
    // static void headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState);
    // static void headerFinished(char c, HttpMessage& message, StateHandler& nextState);

    // Body
    static void finished(char c, HttpMessage& message, StateHandler& nextState);

        static void method(char c, HttpMessage& message, StateHandler& nextState) {
        if (isToken(c)) {
            message.method += c;
        } else if (c == ' ') {
            nextState.func = targetStart;
        } else {
            throw HttpError(400,"Invalid character in method: " + std::string(1, c));
        }
    }

    static void targetStart(char c, HttpMessage& message, StateHandler& nextState) {
        if (c == '/') {
            message.path += c;
            nextState.func = path;
        } else if (isPchar(c)) {
            nextState.func = scheme;
        } else {
            throw HttpError(400, "Invalid character in target: " + std::string(1, c));
        }
    }

    static void scheme(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == ':') {
            nextState.func = colonSlashSlash;
        } else if (!isPchar(c)) {
            throw HttpError(400, "Invalid character in scheme: " + std::string(1, c));
        }
    }

    static void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == ':' || c == '/') {
            return;
        } else if (!isPchar(c)) {
            throw HttpError(400, "Invalid character after scheme: " + std::string(1, c));
        } else {
            nextState.func = authority;
        }
    }

    static void authority(char c, HttpMessage& message, StateHandler& nextState) {
        if (c == ':') {
            nextState.func = port;
        } else if (c == '/') {
            message.path += c;
            nextState.func = path;
        } else if (c == '#') {
            nextState.func = fragment;
        } else if (c == '?') {
            nextState.func = query;
        } else if (!isPchar(c) && c != '.') {
            throw HttpError(400, "Invalid character in authority: " + std::string(1, c));
        }
    }

    static void port(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (isdigit(c)) {
            return;
        } else if (c == '/') {
            nextState.func = path;
        } else if (c == '#') {
            nextState.func = fragment;
        } else if (c == '?') {
            nextState.func = query;
        } else {
            throw HttpError(400, "Invalid character in port: " + std::string(1, c));
        }
    }

    static void path(char c, HttpMessage& message, StateHandler& nextState) {
        if (isPchar(c) || c == '/' || c == '.' || c == ',') {
            message.path += c;
        } else if (c == ' ') {
            nextState.func = httpVersion;
        } else if (c == '?') {
            nextState.func = query;
        } else if (c == '#') {
            nextState.func = fragment;
        } else {
            throw HttpError(400, "Invalid character in path: " + std::string(1, c));
        }
    }

    static void httpVersion(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == 'H') {
            nextState.func = ht;
        } else {
            throw HttpError(400, "Invalid character in HTTP version Start: " + std::string(1, c));
        }
    }

    static void ht(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == 'T') {
            nextState.func = htt;
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void htt(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == 'T') {
            nextState.func = http;
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void http(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == 'P') {
            nextState.func = httpSlash;
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void httpSlash(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '/') {
            nextState.func = majorDigit;
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void majorDigit(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '1') {
            nextState.func = dot;
        } else if (isdigit(c)) {
            throw HttpError(505, "HTTP version not supported");
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void dot(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '.') {
            nextState.func = minorDigit;
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void minorDigit(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '1') {
            nextState.func = CR;
        } else if (isdigit(c)) {
            throw HttpError(505, "HTTP version not supported");
        } else {
            throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
        }
    }

    static void query(char c, HttpMessage& message, StateHandler& nextState) {
        if (isPchar(c) || c == '/' || c == '?') {
            message.query += c;
        } else if (c == '#') {
            nextState.func = fragment;
        } else {
            throw HttpError(400, "Invalid character in query: " + std::string(1, c));
        }
    }

    static void fragment(char c, HttpMessage& message, StateHandler& nextState) {
        if (c == ' ') {
            nextState.func = httpVersion;
        } else if (isPchar(c) || c == '/' || c == '?') {
            message.fragment += c;
        } else {
            throw HttpError(400, "Invalid character in fragment: " + std::string(1, c));
        }
    }

    static void CR(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '\r') {
            nextState.func = NL;
        } else if (c == '\n') {
            nextState.func = fieldName;
        } else {
            throw HttpError(400, "Invalid character in header: " + std::string(1, c));
        }
    }

    static void NL(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '\n') {
            nextState.func = fieldName;
        } else {
            throw HttpError(400, "Invalid character in header: " + std::string(1, c));
        }
    }

    static void fieldName(char c, HttpMessage& message, StateHandler& nextState) {
        if (c == '\r' && message.fieldName.empty()) {
            nextState.func = headerAlmostFinished;
        } else if (c == ':') {
            nextState.func = SP;
        } else if (c == ' ') {
            nextState.func = OWS;
        } else if (isToken(c)) {
            message.fieldName += c;
        } else {
            throw HttpError(400, "Invalid character in field name: " + std::string(1, c));
        }
    }

    static void OWS(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == ':') {
            nextState.func = SP;
        } else {
            throw HttpError(400, "Invalid character after field name" + std::string(1, c));
        }
    }

    static void SP(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == ' ') {
            nextState.func = fieldValue;
        } else {
            throw HttpError(400, "Need space after colon: " + std::string(1, c));
        }
    }

    static void fieldValue(char c, HttpMessage& message, StateHandler& nextState) {
        if (c == '\r') {
            message.headers[message.fieldName] = message.fieldValue;
            message.fieldName = "";
            message.fieldValue = "";
            nextState.func = CR;
        } else if (c == '\n') {
            message.headers[message.fieldName] = message.fieldValue;
            message.fieldName = "";
            message.fieldValue = "";
            nextState.func = fieldValue;
        } else {
            message.fieldValue += c;
        }
    }

    static void headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState) {
        (void) message;
        if (c == '\n') {
            nextState.func = headerFinished;
        } else {
            throw HttpError(400, "Invalid character after headers: " + std::string(1, c));
        }
    }

    static void headerFinished(char c, HttpMessage& message, StateHandler& nextState) {
        (void)c;
        (void)message;
        (void)nextState;
        return;
}
}; // namespace States

    static const size_t MAX_REQUEST_SIZE = 8192;
    static const size_t MAX_HEADER_SIZE = 4096;
    static const size_t MAX_URI_SIZE = 4096;

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

    HttpHeader(const HttpHeader &other);
    std::string percentDecode(std::string &str);
    HttpHeader &operator=(const HttpHeader &other);
};

#endif
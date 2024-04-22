#include "States.hpp"

std::string delimiters = std::string("\"(),/:;<=>?@[\\]{}");

static bool isToken(char c) {
    return (c > 32 && c < 127) && delimiters.find(c) == std::string::npos;
}

std::string subDelis = std::string("!$&'()*+,;=");
static bool isPchar(char c) {
    return isalnum(c) || subDelis.find(c) != std::string::npos || c == ':' || c == '@' || c == '%';
}

namespace States
{
void method(char c, HttpMessage& message, StateHandler& nextState) {
    if (isToken(c)) {
        message.method += c;
    } else if (c == ' ') {
        nextState.func = targetStart;
    } else {
        throw HttpError(414,"Invalid character in method: " + std::string(1, c));
    }
}

void targetStart(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == '/') {
        message.path += c;
        nextState.func = path;
    } else if (isPchar(c)) {
        nextState.func = colonSlashSlash;
    } else {
        throw HttpError(414, "Invalid character in target: " + std::string(1, c));
    }
}

void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (!isPchar(c)) {
        throw HttpError(414, "Invalid character in target: " + std::string(1, c));
    }
    if (c == ':' || c == '/') {
        return;
    } else {
        nextState.func = authority;
    }
}

void scheme(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (isPchar(c)) {
        return;
    } else if (c == ':') {
        nextState.func = authority;
    } else {
        throw HttpError(414, "Invalid character in scheme: " + std::string(1, c));
    }
}

void authority(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (isPchar(c)) {
        return;
    } else if (c == ':') {
        nextState.func = port;
    } else if (c == '/') {
        nextState.func = path;
    } else if (c == '#') {
        nextState.func = fragment;
    } else if (c == '?') {
        nextState.func = query;
    } else {
        throw HttpError(414, "Invalid character in authority: " + std::string(1, c));
    }
}

void port(char c, HttpMessage& message, StateHandler& nextState) {
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
        throw HttpError(414, "Invalid character in port: " + std::string(1, c));
    }
}

void path(char c, HttpMessage& message, StateHandler& nextState) {
    if (isPchar(c)) {
        message.path += c;
    } else if (c == ' ') {
        nextState.func = httpVersion;
    } else if (c == '?') {
        nextState.func = query;
    } else if (c == '#') {
        nextState.func = fragment;
    } else {
        throw HttpError(414, "Invalid character in path: " + std::string(1, c));
    }
}

void httpVersion(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'H') {
        nextState.func = httpSlash;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void ht(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = htt;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void htt(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = http;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void http(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'P') {
        nextState.func = httpSlash;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void httpSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '/') {
        nextState.func = majorDigit;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void majorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = dot;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void dot(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '.') {
        nextState.func = minorDigit;
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void minorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = headerFinished;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(414, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void query(char c, HttpMessage& message, StateHandler& nextState) {
    if (isPchar(c) || c == '/' || c == '?') {
        message.query += c;
    } else if (c == '#') {
        nextState.func = fragment;
    } else {
        throw HttpError(414, "Invalid character in query: " + std::string(1, c));
    }
}

void fragment(char c, HttpMessage& message, StateHandler& nextState) {
    (void) nextState;
    if (c == ' ') {
        nextState.func = httpVersion;
    }
    if (isPchar(c) || c == '/' || c == '?') {
        message.fragment += c;
    } else {
        throw HttpError(414, "Invalid character in fragment: " + std::string(1, c));
    }
}

void headerFinished(char c, HttpMessage& message, StateHandler& nextState) {
    (void)c;
    (void)message;
    (void)nextState;
    return;
}

} // namespace States
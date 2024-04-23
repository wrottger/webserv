#include "States.hpp"
#include <iostream>

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
        throw HttpError(400,"Invalid character in method: " + std::string(1, c));
    }
}

void targetStart(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == '/') {
        message.path += c;
        nextState.func = path;
    } else if (isPchar(c)) {
        nextState.func = scheme;
    } else {
        throw HttpError(400, "Invalid character in target: " + std::string(1, c));
    }
}

void scheme(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':') {
        nextState.func = colonSlashSlash;
    } else if (!isPchar(c)) {
        throw HttpError(400, "Invalid character in scheme: " + std::string(1, c));
    }
}

void colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':' || c == '/') {
        return;
    } else if (!isPchar(c)) {
        throw HttpError(400, "Invalid character after scheme: " + std::string(1, c));
    } else {
        nextState.func = authority;
    }
}

void authority(char c, HttpMessage& message, StateHandler& nextState) {
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
        throw HttpError(400, "Invalid character in port: " + std::string(1, c));
    }
}

void path(char c, HttpMessage& message, StateHandler& nextState) {
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

void httpVersion(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'H') {
        nextState.func = ht;
    } else {
        throw HttpError(400, "Invalid character in HTTP version Start: " + std::string(1, c));
    }
}

void ht(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = htt;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void htt(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = http;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void http(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'P') {
        nextState.func = httpSlash;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void httpSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '/') {
        nextState.func = majorDigit;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void majorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = dot;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void dot(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '.') {
        nextState.func = minorDigit;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void minorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = CR;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void query(char c, HttpMessage& message, StateHandler& nextState) {
    if (isPchar(c) || c == '/' || c == '?') {
        message.query += c;
    } else if (c == '#') {
        nextState.func = fragment;
    } else {
        throw HttpError(400, "Invalid character in query: " + std::string(1, c));
    }
}

void fragment(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == ' ') {
        nextState.func = httpVersion;
    } else if (isPchar(c) || c == '/' || c == '?') {
        message.fragment += c;
    } else {
        throw HttpError(400, "Invalid character in fragment: " + std::string(1, c));
    }
}

void CR(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\r') {
        nextState.func = NL;
    } else if (c == '\n') {
        nextState.func = fieldName;
    } else {
        throw HttpError(400, "Invalid character in header: " + std::string(1, c));
    }
}

void NL(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\n') {
        nextState.func = fieldName;
    } else {
        throw HttpError(400, "Invalid character in header: " + std::string(1, c));
    }
}

void fieldName(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == '\r' && message.fieldName.empty()) {
        nextState.func = headerAlmostFinished;
    } else if (c == ':') {
        nextState.func = SP;
    } else if (c == ' ') {
        nextState.func = OWS;
    } else if (isToken(c)) {
        message.fieldName += tolower(c);
    } else {
        throw HttpError(400, "Invalid character in field name: " + std::string(1, c));
    }
}

void OWS(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':') {
        nextState.func = SP;
    } else {
        throw HttpError(400, "Invalid character after field name" + std::string(1, c));
    }
}

void SP(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ' ') {
        nextState.func = fieldValue;
    } else {
        throw HttpError(400, "Need space after colon: " + std::string(1, c));
    }
}

void fieldValue(char c, HttpMessage& message, StateHandler& nextState) {
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

void headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\n') {
        nextState.func = headerFinished;
    } else {
        throw HttpError(400, "Invalid character after headers: " + std::string(1, c));
    }
}

void headerFinished(char c, HttpMessage& message, StateHandler& nextState) {
    (void)c;
    (void)message;
    (void)nextState;
    return;
}

} // namespace States
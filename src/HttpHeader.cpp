#include <string.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include "Logger.hpp"
#include "HttpHeader.hpp"
#include "HttpError.hpp"

HttpHeader::HttpHeader() : parseError(0, ""){
    complete = false;
    state = new StateHandler();
    state->func = States::method;
    message = HttpMessage();
    request_size = 0;
}

HttpHeader::~HttpHeader() { delete state; }

size_t HttpHeader::parseBuffer(const char *requestLine) {
    if (requestLine == NULL || isComplete() || parseError.code() != 0)
        return 0;
    char c;
    size_t i = 0;
    for (; requestLine[i] != '\0' && state->func != States::headerFinished; i++)
    {
        request_size++;
        if (request_size > 8192) // TODO check against config
        {
            LOG_INFO_WITH_TAG("Request Entity Too Large", "HttpHeader::parseBuffer");
            throw HttpError(413, "Request Entity Too Large");
        }
        c = requestLine[i];
        try
        {
            state->func(c, message, *state);
        }
        catch(HttpError &e)
        {
            parseError = e;
            return i;
        }
    }
    if (state->func == States::headerFinished)
    {
        LOG_DEBUG_WITH_TAG("header parsing finished", "HttpHeader::parseBuffer");
        message.path = percentDecode(message.path);
        complete = true;
        if (message.headers.count("host") == 0)
        {
            // print headers
            for (auto &header : message.headers)
            {
                std::cout << header.first << ": " << header.second << std::endl;
            }
            LOG_INFO_WITH_TAG("host header not found", "HttpHeader::parseBuffer");
            parseError = HttpError(400, "Host header is required");
        }
        if (message.headers.find("host")->second.find(":") != std::string::npos)
        {
            message.headers["host"] = message.headers.find("host")->second.substr(0, message.headers.find("host")->second.find(":"));
        }
    }
    return i;
}

const std::map<std::string, std::string> &HttpHeader::getHeaders() const {
	return message.headers;
}

const std::string &HttpHeader::getMethod() const { return message.method; }

const std::string &HttpHeader::getPath() const { return message.path; }

const std::string &HttpHeader::getQuery() const {  return message.query; }

const std::string HttpHeader::getHeader(const std::string &name) const {
    std::map<std::string, std::string>::const_iterator it = message.headers.find(name);
    if (it == message.headers.end())
        return "";
    return it->second;
}

bool HttpHeader::isError() const { return parseError.code() != 0;}
HttpError HttpHeader::getError() const { return parseError; }

std::string HttpHeader::percentDecode(std::string &str)
{
    std::string decoded;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == '%')
        {
            if (i + 2 >= str.size())
            {
                parseError = HttpError(400, "Invalid percent encoding");
                return "";
            }
            char c = (str[i + 1] - '0') * 16 + (str[i + 2] - '0');
            decoded.push_back(c);
            i += 2;
        }
        else
        {
            decoded.push_back(str[i]);
        }
    }
    return decoded;
}

bool HttpHeader::isComplete() const { return state->func == States::headerFinished; }


bool HttpHeader::States::isToken(char c) {
    const std::string delimiters = std::string("\"(),/:;<=>?@[\\]{}");
    return (c > 32 && c < 127) && delimiters.find(c) == std::string::npos;
}

bool HttpHeader::States::isPchar(char c) {
    const static std::string subDelis = std::string("!$&'()*+,;=");
    return isalnum(c) || subDelis.find(c) != std::string::npos || c == ':' || c == '@' || c == '%';
}

// Body
void HttpHeader::States::method(char c, HttpMessage& message, StateHandler& nextState) {
    if (isToken(c)) {
        message.method += c;
    } else if (c == ' ') {
        nextState.func = targetStart;
    } else {
        throw HttpError(400,"Invalid character in method: " + std::string(1, c));
    }
}

void HttpHeader::States::targetStart(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == '/') {
        message.path += c;
        nextState.func = path;
    } else if (isPchar(c)) {
        nextState.func = scheme;
    } else {
        throw HttpError(400, "Invalid character in target: " + std::string(1, c));
    }
}

void HttpHeader::States::scheme(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':') {
        nextState.func = colonSlashSlash;
    } else if (!isPchar(c)) {
        throw HttpError(400, "Invalid character in scheme: " + std::string(1, c));
    }
}

void HttpHeader::States::colonSlashSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':' || c == '/') {
        return;
    } else if (!isPchar(c)) {
        throw HttpError(400, "Invalid character after scheme: " + std::string(1, c));
    } else {
        nextState.func = authority;
    }
}

void HttpHeader::States::authority(char c, HttpMessage& message, StateHandler& nextState) {
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

void HttpHeader::States::port(char c, HttpMessage& message, StateHandler& nextState) {
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

void HttpHeader::States::path(char c, HttpMessage& message, StateHandler& nextState) {
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

void HttpHeader::States::httpVersion(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'H') {
        nextState.func = ht;
    } else {
        throw HttpError(400, "Invalid character in HTTP version Start: " + std::string(1, c));
    }
}

void HttpHeader::States::ht(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = htt;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::htt(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'T') {
        nextState.func = http;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::http(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == 'P') {
        nextState.func = httpSlash;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::httpSlash(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '/') {
        nextState.func = majorDigit;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::majorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = dot;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::dot(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '.') {
        nextState.func = minorDigit;
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::minorDigit(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '1') {
        nextState.func = CR;
    } else if (isdigit(c)) {
        throw HttpError(505, "HTTP version not supported");
    } else {
        throw HttpError(400, "Invalid character in HTTP version: " + std::string(1, c));
    }
}

void HttpHeader::States::query(char c, HttpMessage& message, StateHandler& nextState) {
    if (isPchar(c) || c == '/' || c == '?') {
        message.query += c;
    } else if (c == '#') {
        nextState.func = fragment;
    } else {
        throw HttpError(400, "Invalid character in query: " + std::string(1, c));
    }
}

void HttpHeader::States::fragment(char c, HttpMessage& message, StateHandler& nextState) {
    if (c == ' ') {
        nextState.func = httpVersion;
    } else if (isPchar(c) || c == '/' || c == '?') {
        message.fragment += c;
    } else {
        throw HttpError(400, "Invalid character in fragment: " + std::string(1, c));
    }
}

void HttpHeader::States::CR(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\r') {
        nextState.func = NL;
    } else if (c == '\n') {
        nextState.func = fieldName;
    } else {
        throw HttpError(400, "Invalid character in header: " + std::string(1, c));
    }
}

void HttpHeader::States::NL(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\n') {
        nextState.func = fieldName;
    } else {
        throw HttpError(400, "Invalid character in header: " + std::string(1, c));
    }
}

void HttpHeader::States::fieldName(char c, HttpMessage& message, StateHandler& nextState) {
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

void HttpHeader::States::OWS(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ':') {
        nextState.func = SP;
    } else {
        throw HttpError(400, "Invalid character after field name" + std::string(1, c));
    }
}

void HttpHeader::States::SP(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == ' ') {
        nextState.func = fieldValue;
    } else {
        throw HttpError(400, "Need space after colon: " + std::string(1, c));
    }
}

void HttpHeader::States::fieldValue(char c, HttpMessage& message, StateHandler& nextState) {
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

void HttpHeader::States::headerAlmostFinished(char c, HttpMessage& message, StateHandler& nextState) {
    (void) message;
    if (c == '\n') {
        nextState.func = headerFinished;
    } else {
        throw HttpError(400, "Invalid character after headers: " + std::string(1, c));
    }
}

void HttpHeader::States::headerFinished(char c, HttpMessage& message, StateHandler& nextState) {
    (void)c;
    (void)message;
    (void)nextState;
    return;
}


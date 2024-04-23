#include <string.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "HttpMessage.hpp"

HttpRequest::HttpRequest() : parseError(0, ""){
    complete = false;
    state = new StateHandler();
    state->func = &States::method;
    message = HttpMessage();
    request_size = 0;
}

HttpRequest::~HttpRequest() { delete state; }

size_t HttpRequest::parseBuffer(const char *requestLine) {
    if (requestLine == NULL || isComplete() || parseError.code() != 0)
        return 0;
    char c;
    size_t i = 0;
    for (; requestLine[i] != '\0' && state->func != States::headerFinished; i++)
    {
        request_size++;
        if (request_size > 8192)
            throw HttpError(413, "Request Entity Too Large");
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
        message.path = percentDecode(message.path);
        complete = true;
        if (headers.count("host") == 0)
            parseError = HttpError(400, "Host header is required");
    }
    return i;
}

const std::string &HttpRequest::getMethod() const { return message.method; }

const std::string &HttpRequest::getPath() const { return message.path; }

const std::string &HttpRequest::getQuery() const {  return message.query; }

const std::string &HttpRequest::getHeader(const std::string &name) const {
    return message.headers.find(name)->second;
}

bool HttpRequest::isError() const { return parseError.code() != 0;}
HttpError HttpRequest::getError() const { return parseError; }

std::string HttpRequest::percentDecode(std::string &str)
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

bool HttpRequest::isComplete() const { return state->func == States::headerFinished; }

// size_t HttpRequest::parse_header(const char *requestLine)
// {
//     return size_t();
// }

// size_t HttpRequest::parse_body(const char *requestLine)
// {
//     return size_t();
// }

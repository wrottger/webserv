#include <string.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "HttpMessage.hpp"

HttpRequest::HttpRequest() {
    headerComplete = false;
    complete = false;
    state = new StateHandler();
    state->func = &States::method;
    message = HttpMessage();
    request_size = 0;
}

HttpRequest::~HttpRequest() { delete state; }

size_t HttpRequest::parseBuffer(const char *requestLine) {
    if (requestLine == NULL || isComplete())
        return 0;
    char c;
    size_t i = 0;
    for (; requestLine[i] != '\0'; i++)
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
            std::cerr << e.what() << "  " << e.code() << '\n';
        }
        if (state->func == States::headerFinished)
            headerComplete = true;
    }
    std::cerr << request_size << std::endl;

    // TODO percent decode path
    return i;
}

const std::string &HttpRequest::getMethod() const { return message.method; }

const std::string &HttpRequest::getPath() const { return message.path; }

const std::string &HttpRequest::getHeader(const std::string &name) const {
    return message.headers.find(name)->second;
}

bool HttpRequest::isComplete() const { return state->func == States::headerFinished; }

const std::string &HttpRequest::getBody() const { return message.body; }

bool HttpRequest::isToken(char c)
{
    if (32 <= c && c <= 126)
        return true;
    return false;
}
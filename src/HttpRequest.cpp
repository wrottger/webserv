#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include <string.h>
#include <algorithm>
#include <iostream>

HttpRequest::HttpRequest() {
    // Initialize member variables
    body = "";
    target = "";
    query = "";
    version = "";
    state = s_start;
    headers.clear();
    request_size = 0;
}

size_t HttpRequest::parseBuffer(const char *requestLine) {
    if (requestLine == NULL)
        return 0;
    char c;
    for (size_t i = 0; requestLine[i] != '\0'; i++)
    {
        request_size++;
        if (request_size > 8192)
            throw HttpError(413, "Request Entity Too Large");
        c = requestLine[i];
        switch (s_method)
        {
        case s_start:
            if (isToken(c))
                state = s_method;
            else
                throw HttpError(400, "Bad Request");            
            break;
        case s_method:
            if (method.size() > 7)
                throw HttpError(501, "Not Implemented");
            if (isToken(c))
                method.push_back(c);
            else if (c == ' ')
                state = s_spaces_before_uri;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_spaces_before_uri:
            if (c == ' ') {
                state = s_after_first_space;
            } else if (c == '/') {
                state = s_uri;  // Origin form
            } else if (c == 'h') {
                state = s_schema_start;  // Possible absolute form
            } else {
                throw HttpError(400, "Bad Request");
            }
            break;
        case s_after_first_space:
            if (c == ' ') {
                throw HttpError(400, "Bad Request: Multiple spaces before URI");
            } else if (c == '/') {
                state = s_uri;  // Origin form
            } else if (c == 'h') {
                state = s_schema_start;  // Possible absolute form
            } else {
                throw HttpError(400, "Bad Request");
            }
            break;
        case s_schema:

        case s_host_start:
        case s_host:
        case s_host_end:
        case s_host_ip_literal:
        case s_after_slash_in_uri:
        case s_check_uri:
        case s_uri:
        case s_http_09:
        case s_http_H:
        case s_http_HT:
        case s_http_HTT:
        case s_http_HTTP:
        case s_first_major_digit:
        case s_major_digit:
        case s_first_minor_digit:
        case s_minor_digit:
        case s_spaces_after_digit:
        case s_almost_done:
        case s_finished:
            break;
        default:
            throw HttpError(500, "Internal Server Error");
        }
    }

    return 0;
}

const std::string &HttpRequest::getMethod() const { return method; }

const std::string &HttpRequest::getTarget() const { return target; }

const std::string &HttpRequest::getQuery() const { return query; }

const std::string &HttpRequest::getVersion() const { return version; }

const std::string &HttpRequest::getHeader(const std::string &name) const {
    return headers.at(name);
}

const std::string &HttpRequest::getBody() const {
    return body;
}

// bool HttpRequest::isComplete() const
// {
//     return state == s_finished;
// }

// size_t HttpRequest::parse_header(const char *requestLine)
// {
//     return size_t();
// }

// size_t HttpRequest::parse_body(const char *requestLine)
// {
//     return size_t();
// }

bool HttpRequest::isToken(char c)
{
    if (32 <= c && c <= 126)
        return true;
    return false;
}
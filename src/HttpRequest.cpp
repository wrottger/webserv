#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include <string.h>
#include <algorithm>
#include <iostream>

HttpRequest::HttpRequest() {
    state = s_method;
    request_size = 0;
}

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
        switch (s_method)
        {
        case s_method:
            if (method.size() > 7)
                throw HttpError(501, method + " method not Implemented");
            if (c == ' ')
                state = s_uri;
            else if (isToken(c))
                method.push_back(c);
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_uri:
            if (c == '/')
            {
                state = s_host_start;
                target.push_back(c);
            }
            else if (tolower(c) == 'h')
                state = s_schema;
            else if (c == ' ')
                throw HttpError(400, "Bad Request");
            break;
        case s_host_start:
            if (c == ' ')
                state = s_http_H;
            else if (c == '?')
                state = s_query;
            else if (c == '#')
                state = s_fragment;
            else if (target.size() > MAX_URI_SIZE)
                throw HttpError(414, "URI Too Long");
            else
                target.push_back(c);
            break;
        case s_query:
            if (c == ' ')
                state = s_http_H;
            else if (c == '#')
                state = s_fragment;
            else if (target.size() + query.size() > MAX_URI_SIZE)
                throw HttpError(414, "URI Too Long");
            else
                query.push_back(c);
            break;
        case s_fragment:
            if (c == ' ')
                state = s_http_H;
            else if (target.size() + query.size() + fragment.size() > MAX_URI_SIZE)
                throw HttpError(414, "URI Too Long");
            else
                fragment.push_back(c);
            break;
        case s_schema:
            if (c == ':')
            {
                if (schema == "https")
                    throw HttpError(501, "HTTPS not implemented");
                if (schema != "http")
                    throw HttpError(400, "Bad Request");
                state = s_host_start;
            }
            schema.push_back(c);
            if (!isToken(c))
                throw HttpError(400, "Bad Request");
            if (schema.size() > 4)
                throw HttpError(400, "Bad Request");
            break;
        case s_host:
            if (c == ' ')
                state = s_http_H;
            else if (c == ' ')
                state = s_host_ip_literal;
            else
                host.push_back(c);
            break;
        case s_host_ip_literal:
        case s_check_uri:
        case s_http_H:
            if (c == 'H')
                state = s_http_HT;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_http_HT:
            if (c == 'T')
                state = s_http_HTT;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_http_HTT:
            if (c == 'T')
                state = s_http_HTTP;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_http_HTTP:
            if (c == 'P')
                state = s_slash_after_HTTP;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_slash_after_HTTP:
            if (c == '/')
                state = s_first_major_digit;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_first_major_digit:
            if (c == '1')
                state = s_major_digit;
            else if (isdigit(c))
                throw HttpError(505, "HTTP Version Not Supported");
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_major_digit:
            if (c == '.')
                state = s_first_minor_digit;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_first_minor_digit:
            if (c == '1')
                state = s_minor_digit;
            else if (isdigit(c))
                throw HttpError(505, "HTTP Version Not Supported");
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_minor_digit:
            if (c == '\r')
                state = s_field_name;
            else if (c == '\n')
                state = s_headers;
            else if (isdigit(c))
                throw HttpError(505, "HTTP Version Not Supported");
            else 
                throw HttpError(400, "Bad Request");
            break;
        case s_spaces_after_digit:
        case s_almost_done:
            if (c == '\n')
                state = s_field_name;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_field_name:
            if (fieldName.size() > MAX_HEADER_SIZE)
                throw HttpError(431, "Request Header Fields Too Large");
            if (c == ':')
                state = s_field_value;
            else if (!isToken(c))
                throw HttpError(400, "Bad Request");
            fieldName.push_back(c);
            break;
        case s_field_value:
            if (fieldValue.size() + fieldName.size() > MAX_HEADER_SIZE)
                throw HttpError(431, "Request Header Fields Too Large");
            if (c == '\r')
                state = s_header_almost_end;
            if (c == '\n')
                state = s_header_end;
            fieldValue.push_back(c);
            break;
        case s_header_almost_end:
            if (c == '\n')
                state = s_header_end;
            else
                throw HttpError(400, "Bad Request");
            break;
        case s_header_end:
            if (c == '\r')
                state = s_finished;
            else if (isToken(c))
            {
                headers[fieldName] = fieldValue;
                fieldName.clear();
                fieldValue.clear();
                fieldName.push_back(c);
                state = s_field_name;
            }
            else
                throw HttpError(400, "Bad Request");
            break;
        default:
            throw HttpError(500, "Internal Server Error");
        }
    }
    return i;
}

const std::string &HttpRequest::getMethod() const { return method; }

const std::string &HttpRequest::getTarget() const { return target; }

const std::string &HttpRequest::getHeader(const std::string &name) const {
    return headers.find(name)->second;
}

bool HttpRequest::isComplete() const { return state == s_finished; }

const std::string &HttpRequest::getBody() const { return body; }

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
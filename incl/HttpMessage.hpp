#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP

#include <string>
#include <map>

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

#endif // HTTPMESSAGE_HPP

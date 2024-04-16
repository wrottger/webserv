#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest();
    size_t parseBuffer(const char *requestLine);
    const std::string &getMethod() const;
    const std::string &getTarget() const;
    const std::string &getQuery() const;
    const std::string &getHeader(const std::string &name) const;
    const std::string &getBody() const;

    bool isComplete() const; //TODO add seperate headerComplete and bodyComplete

private:
    static const size_t MAX_REQUEST_SIZE = 8192;
    static const size_t MAX_HEADER_SIZE = 4096;
    static const size_t MAX_URI_SIZE = 4096;
    enum State {
        s_start = 0,
        s_method,
        s_spaces_before_uri,
        s_schema,
        s_after_first_space,
        s_host_start,
        s_host,
        s_host_end,
        s_host_ip_literal,
        s_query,
        s_fragment,
        s_headers,
        s_header_almost_end,
        s_header_end,
        s_port,
        s_after_slash_in_uri,
        s_check_uri,
        s_uri,
        s_http_09,
        s_http_H,
        s_http_HT,
        s_http_HTT,
        s_http_HTTP,
        s_slash_after_HTTP,
        s_first_major_digit,
        s_field_name,
        s_field_value,
        s_major_digit,
        s_first_minor_digit,
        s_minor_digit,
        s_spaces_after_digit,
        s_almost_done,
        s_finished
    };

    State state;
    size_t request_size;

    std::string method; // GET, POST etc.
    std::string host; // localhost, google.com etc.
    std::string target; // /, /index.html etc.
    std::string query; // ?key=value etc.
    std::string fragment; // #fragment etc.
    std::map<std::string, std::string> headers;

    std::string body;

    std::string schema;
    std::string fieldName;
    std::string fieldValue;

    HttpRequest(const HttpRequest &other);
    HttpRequest &operator=(const HttpRequest &other);

    static bool isToken(char c);
};

#endif
#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest();
    size_t parseLine(const char *requestLine);
    const std::string &getMethod() const;
    const std::string &getTarget() const;
    const std::string &getQuery() const;
    const std::string &getVersion() const;
    const std::string &getHeader(const std::string &name) const;
    const std::string &getBody() const;

    bool isComplete() const;

private:
    enum State {
        s_start = 0,
        s_method,
        s_spaces_before_uri,
        s_schema,
        s_schema_slash,
        s_schema_slash_slash,
        s_host_start,
        s_host,
        s_host_end,
        s_host_ip_literal,
        s_port,
        s_after_slash_in_uri,
        s_check_uri,
        s_uri,
        s_http_09,
        s_http_H,
        s_http_HT,
        s_http_HTT,
        s_http_HTTP,
        s_first_major_digit,
        s_major_digit,
        s_first_minor_digit,
        s_minor_digit,
        s_spaces_after_digit,
        s_almost_done,
        s_finished
    };

    State state;
    char method_buf[10];
    std::string method;
    std::string buf;
    std::string target;
    std::string query;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t request_size;

    // static size_t parse_header(const char *requestLine);
    // static size_t parse_body(const char *requestLine);
    static bool isToken(char c);
};

#endif
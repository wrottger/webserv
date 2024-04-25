#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HttpHeader.hpp"

class HttpResponse
{
    public:
        HttpResponse(HttpHeader &header, int fds);
        ~HttpResponse();
        size_t readBuffer(const char* buffer);
        void write();
        bool finished();

    private:
        HttpResponse();
        HttpResponse(const HttpResponse &src);
        HttpResponse &operator=(const HttpResponse &src);
};
#endif // HTTPRESPONSE_HPP

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HttpHeader.hpp"
#include "Config.hpp"

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

        HttpHeader & header;
        Config *config;
        int fds;
        HttpError error;
        bool isChunked;
        bool isFinished;
        std::string response;
};
#endif // HTTPRESPONSE_HPP

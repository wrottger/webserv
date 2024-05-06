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
		std::string generateErrorResponse(int code, const std::string &message);

        HttpHeader & header;
        Config *config;
        int fds;
        HttpError error;
        bool isChunked;
        bool isFinished;
        std::string response;

        // CHUNKED response
        std::ifstream getFile;
        char chunkedBuffer[1025] = {};
        std::string chunks;
        size_t bufferIndex;
};
#endif // HTTPRESPONSE_HPP

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HttpHeader.hpp"
#include "Config.hpp"

class HttpResponse
{
    public:
        HttpResponse();
		HttpResponse(HttpHeader header, int fds);
        size_t readBuffer(const char* buffer);
        void write();
        bool finished();
        bool isBodyFinished();

    private:
        HttpResponse operator=(HttpResponse other);
        HttpResponse(const HttpResponse&);
		void generateDirListing();
		std::string generateErrorResponse(const HttpError &error);
		HttpError setupGetResponse();

        HttpHeader header;
        Config &config;
        std::string host;
        std::string path;
        int fds;
        bool isChunked;
        bool isFinished;
        bool isError;
        bool bodyFinished;
        std::string response;

        // CHUNKED response
        std::ifstream getFile;
        char chunkedBuffer[1025];
        std::string chunks;
        size_t bufferIndex;

        struct fileInfo {
            std::string name;
            std::string size;
            std::string date;
        } typedef fileInfo;
        int listDir(std::string dir, std::vector<fileInfo> &files);
};

#endif // HTTPRESPONSE_HPP

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
		void generateDirListing();
        HttpResponse();
        HttpResponse(const HttpResponse &src);
        HttpResponse &operator=(const HttpResponse &src);
		std::string generateErrorResponse(const HttpError &error);

        HttpHeader & header;
        Config *config;
        int fds;
        bool isChunked;
        bool isFinished;
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

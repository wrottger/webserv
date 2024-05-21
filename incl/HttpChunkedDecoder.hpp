#ifndef HTTP_CHUNKED_DECODER_HPP
#define HTTP_CHUNKED_DECODER_HPP

#include <string>
#include <sstream>

class HttpChunkedDecoder {

    private:
        enum State {
            READ_SIZE,
            READ_SIZE_END,
            READ_CHUNK,
            READ_TRAILER_CR,
            READ_TRAILER_LF
        };
    
    static HttpChunkedDecoder::State state;
    static std::stringstream ss;
    static bool lastChunk;
    static unsigned int chunkSize;

    public:
        HttpChunkedDecoder();
        int decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody);
};

#endif
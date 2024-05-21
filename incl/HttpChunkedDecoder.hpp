#ifndef HTTP_CHUNKED_DECODER_HPP
#define HTTP_CHUNKED_DECODER_HPP

#include <string>
#include <sstream>
#include <vector>
#include "Logger.hpp"

class HttpChunkedDecoder {

    private:
        enum State {
            READ_SIZE,
            READ_SIZE_END,
            READ_CHUNK,
            READ_TRAILER_CR,
            READ_TRAILER_LF
        };
    
        State state;
        std::stringstream ss;
        bool lastChunk;
        unsigned int chunkSize;

    public:
        HttpChunkedDecoder();
        int decodeChunkedBody(std::vector<char> &bodyBuffer, std::vector<char> &decodedBody);
        size_t getChunkSize() const;
};

#endif
#include "HttpChunkedDecoder.hpp"

HttpChunkedDecoder::HttpChunkedDecoder() 
{
    state = READ_SIZE;
    lastChunk = false;
    chunkSize = 0;
    ss.str("");
}

// description: Decodes the chunked body and stores it in decodedBody until chunk size is 0;
// returns 0 if the last chunk is reached, 1 if more data is needed, -1 if an error occurs
int HttpChunkedDecoder::decodeChunkedBody(std::string &bodyBuffer, std::string &decodedBody)
{
    if (lastChunk)
        return 0; // More data will not be processed
    for (size_t i = 0; i < bodyBuffer.size(); ++i)
    {
        switch (state)
        {
            case READ_SIZE:
            {
                if (bodyBuffer[i] == '\r')
                    state = READ_SIZE_END;
                else if (isxdigit(bodyBuffer[i]))
                    ss << bodyBuffer[i];
				else
					return -1;
                break;
            }
            case READ_SIZE_END:
            {
                if (bodyBuffer[i] == '\n')
                {
                    if (!(ss >> std::hex >> chunkSize) || !ss.eof())
                        return -1; // string to hex conversion failed
					ss.str("");
                    ss.clear();
                    if (chunkSize == 0)
                        lastChunk = true;
                    state = READ_CHUNK;
                }
                else
                    return -1;
                break;
            }
            case READ_CHUNK:
            {
                decodedBody.push_back(bodyBuffer[i]);
                chunkSize--;
                if (chunkSize == 0)
                    state = READ_TRAILER_CR;
                break;
            }
            case READ_TRAILER_CR:
            {
                if (bodyBuffer[i] == '\r')
                    state = READ_TRAILER_LF;
                else
                    return -1; // Invalid trailer
                break;
            }
            case READ_TRAILER_LF:
            {
                if (bodyBuffer[i] == '\n')
                {
                    if (lastChunk)
                        return 0; // Finished decoding
                    state = READ_SIZE;
                }
                else
                    return -1; // Invalid trailer
                break;
            }
        }
    }
    return 1; // More data needed
}

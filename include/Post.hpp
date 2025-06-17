#ifndef POST_HPP
#define POST_HPP

#include <fstream>
class Post
{
public:
    enum  TransferType {
    CONTENT_LENGTH,
    CHUNKED
    };
    enum  ChunkState {
        READING_CHUNK_SIZE,
        READING_CHUNK_SIZE_CRLF,
        READING_CHUNK_DATA,
        READING_CHUNK_DATA_CRLF,
        READING_TRAILER_HEADERS,
        READING_FINAL_CRLF,
        CHUNK_COMPLETE,
        CHUNK_ERROR          
    };
    Post(std::string &);
    ~Post();
    void ProcessChunck();
    void ProcessContentLength();
private:
    TransferType transfer_type;
    ChunkState chunk_state;
    std::ofstream OutputFile;
};
#endif
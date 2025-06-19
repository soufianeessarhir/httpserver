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
    Post();
    ~Post();
    void SetTransferType(TransferType );
    void SetChunkState(ChunkState);
    void ProcessChunck();
    void ProcessContentLength();
private:
    TransferType transfer_type;
    ChunkState chunk_state;

    // Chunk-specific data
    std::string chunk_size_buffer;
    size_t current_chunk_size;
    size_t chunk_bytes_read;
    std::string chunk_data_buffer;
    
    // Content-Length specific data
    size_t content_length;
    size_t content_bytes_read;
    
    // Common data
    std::string body_buffer;
    std::ofstream output_file;
    size_t max_body_size;
};
#endif
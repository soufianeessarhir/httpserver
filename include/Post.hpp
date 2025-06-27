#ifndef POST_HPP
#define POST_HPP

#include <fstream>
#include "Connection.hpp"
#include "MultiPart.hpp"
class Connection;
class Post
{
public:
    enum  TransferType {
    CONTENT_LENGTH,
    CHUNKED,
    ERROR
    };
    enum  ChunkState {
        READING_CHUNK_SIZE,
        READING_CHUNK_DATA,
        READING_TRAILER_HEADERS,
        CHUNK_COMPLETE,
        CHUNK_ERROR          
    };
    enum MultiPaertState
    {
        READING_PREAMBLE,       
        READING_BOUNDARY,       
        READING_PART_HEADERS,   
        READING_PART_DATA,
        READING_EPILOGUE,
        MULTIPART_COMPLETE,
        MULTIPART_ERROR   
    };
    Post(Connection * , TransferType);
    ~Post();
    void ProcessChunck();
    void ProcessContentLength();
    TransferType transfer_type;

private:
    ChunkState chunk_state;
    Connection *conn;
    std::string chunk_size_buffer;
    size_t current_chunk_size;
    size_t chunk_bytes_read;
    std::string chunk_data_buffer;
    size_t content_length;
    size_t content_bytes_read;
    std::ofstream output_file;
    size_t max_body_size;
    bool is_multipart;
    std::string boundry;
    std::vector<MultiPart> parts;
    MultiPaertState multipart_state;


private:
    void ReadChunkSize();
    void ReadChunkData();
    void ReadTrailerHeaders();
    void ProcessMultiPart();
    bool ExtractAndValidateBoundry();
};
#endif
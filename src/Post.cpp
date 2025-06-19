#include "Post.hpp"

Post::Post()
{

}

void Post::ProcessChunck()
{
    switch (chunk_state)
    {
        case Post::READING_CHUNK_SIZE:
            break;
        case Post::READING_CHUNK_SIZE_CRLF:
            break;
        case Post::READING_CHUNK_DATA:
            break;
        case Post::READING_CHUNK_DATA_CRLF:
            break;
        case Post::READING_TRAILER_HEADERS:
            break;
        case Post::READING_FINAL_CRLF:
            break;
        case Post::CHUNK_COMPLETE:
            break;
        case Post::CHUNK_ERROR:
            break;
    }
}

void Post::SetTransferType(TransferType type)
{
    transfer_type = type;
}
void Post::SetChunkState(ChunkState state)
{
    chunk_state = state;
}

void Post::ProcessContentLength()
{

}

Post::~Post()
{
    if (output_file.is_open())
    {
        output_file.close();
    }
}
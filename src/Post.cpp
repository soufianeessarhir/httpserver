#include "Post.hpp"

Post::Post(Connection *conn , TransferType type):conn(conn)
{
    if (type ==  CONTENT_LENGTH)
    {
        content_length = conn->request->GetContentLenght();
        content_bytes_read = 0;
    }
    else
    {
        chunk_state = READING_CHUNK_SIZE;
        chunk_bytes_read = 0;
    }
    max_body_size = conn->location->max_body_size;

}

void Post::ProcessChunck()
{
    switch (chunk_state)
    {
        case Post::READING_CHUNK_SIZE:
            break;
        case Post::READING_CHUNK_DATA:
            break;
        case Post::READING_TRAILER_HEADERS:
            break;
        case Post::CHUNK_COMPLETE:
            break;
        case Post::CHUNK_ERROR:
            break;
    }
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
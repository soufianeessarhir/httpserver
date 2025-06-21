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
void Post::ReadChunkSize()
{
    size_t CRLF = conn->buffer.find("\r\n");
    if (CRLF == std::string::npos)
        return;
    std::string size_str = conn->buffer.substr(0,CRLF);
    conn->buffer.erase(0,CRLF + 2);
    size_t i = 0;
    while (i < size_str.size() && isxdigit(static_cast<char>(size_str[i])))
        i++;
    errno = 0;
    char *endp =NULL;
    current_chunk_size = std::strtol(size_str.c_str(),&endp,16);
    if (errno == ERANGE || !endp /*my need to check for large size*/)
    {
        chunk_state = Post::CHUNK_ERROR;
        return;
    }
    if (current_chunk_size == 0)
        chunk_state = Post::READING_TRAILER_HEADERS;
    chunk_state = Post::READING_CHUNK_DATA;
}

void Post::ReadChunkData()
{

}

void Post::ReadTrailerHeaders()
{

}

void Post::ProcessChunck()
{
    switch (chunk_state)
    {
        case Post::READING_CHUNK_SIZE:
            ReadChunkSize();
            break;
        case Post::READING_CHUNK_DATA:
            ReadChunkData();
            break;
        case Post::READING_TRAILER_HEADERS:
            ReadTrailerHeaders();
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
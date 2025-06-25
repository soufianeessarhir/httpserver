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
    std::string content_type = conn->request->GetHeader("content-Type");
    if (content_type.find("multipart/form-data") != std::string::npos)
    {
        is_multipart = true;
        
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
    size_t size_to_read = std::min(conn->buffer.size(),current_chunk_size - chunk_bytes_read);
    //porcess readed data
    conn->buffer.erase(0,size_to_read);
    if (current_chunk_size == chunk_bytes_read)
    {
        size_t CRLF = conn->buffer.find("\r\n");
        if (CRLF != std::string::npos)
        {
            conn->buffer.erase(0,2);
            chunk_state = Post::READING_CHUNK_SIZE;
            current_chunk_size = 0;
            chunk_bytes_read = 0;
        }
        else
        {
            chunk_state = Post::CHUNK_ERROR;
        }
    }
}

void Post::ReadTrailerHeaders()
{
    // should be protected for size limits
    size_t CRLF = conn->buffer.find("\r\n");
    if (CRLF != std::string::npos)
    {
        conn->buffer.erase(0,CRLF + 2);
        chunk_state = Post::CHUNK_COMPLETE;
    }
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
            conn->state = Connection::SENDING_RESPONSE;
            break;
        case Post::CHUNK_ERROR:
            //the status code should be set here
            conn->state = Connection::SENDING_RESPONSE;
            break;
    }
}

void Post::ProcessMultiPart()
{
    std::string delimiter = "--" + boundry;
    switch (multipart_state)
    {
        case READING_PREAMBLE:
        {
            size_t del = conn->buffer.find(delimiter);
            if (del != std::string::npos)
            {
                conn->buffer.erase(0,del);
                multipart_state = Post::READING_BOUNDARY;
            }
        }
        break;
        case READING_BOUNDARY:
        {
            size_t CRLF = conn->buffer.find("\r\n");
            if(CRLF != std::string::npos)
            {
                if (conn->buffer.substr(0,CRLF) != delimiter)
                {
                    multipart_state = Post::MULTIPART_ERROR;
                }
                conn->buffer.erase(0,CRLF + 2);
                multipart_state = Post::READING_PART_HEADERS;
            }
        }
        break;
        case READING_PART_HEADERS:
            /* code */
            break;
        case READING_PART_DATA:
            /* code */
            break;
        case READING_EPILOGUE:
            /* code */
            break;
        case MULTIPART_COMPLETE:
            /* code */
            break;
        case MULTIPART_ERROR:
            /* code */
            break;
        }
}
void Post::ProcessContentLength()
{
    size_t bytes_to_read = std::min(conn->buffer.size(),content_length - content_bytes_read);
    //write to file 
    conn->buffer.erase(0,bytes_to_read);
    content_bytes_read += bytes_to_read;
}

Post::~Post()
{
    if (output_file.is_open())
    {
        output_file.close();
    }
}
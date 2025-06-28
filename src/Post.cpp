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
        multipart_state= Post::READING_PREAMBLE;
        if (!ExtractAndValidateBoundry())
        {
            transfer_type = Post::ERROR;
        }
        
    }          
    max_body_size = conn->location->max_body_size;

}

bool Post::ExtractAndValidateBoundry()
{
    std::string ct = conn->request->GetHeader("content-type");
    if (ct.empty())
        return false;
    std::string origin = ct;
    for (std::string::iterator it = ct.begin();it != ct.end();++it)
        *it = std::tolower(static_cast<char>(*it)); 
    size_t BNDR = ct.find("boundary");
    if (BNDR==std::string::npos)
        return false;
    size_t begin = BNDR + 9;
    for (;begin < ct.length() && isspace(ct[begin]);++begin);
    if (ct[begin] != '=')
    return false;
    begin++;
    for (;begin < ct.length() && isspace(ct[begin]);++begin);

    size_t end;
    if (ct[begin] == '"')
    {
        begin ++;
        size_t q = ct.find(begin,'"');
        if (q != std::string::npos)
        {
            boundry = origin.substr(begin,q - begin);
        }
    }
    else {
        for (end=begin;end < ct.length()&&
        ct[end] != ' ' && ct[end] != '\t' && 
        ct[end] != ';' && ct[end] != '\r' && ct[end] != '\n';++end );
        if (end >= ct.length())
            return false;
        boundry = origin.substr(begin, end - begin);
    }
    if (boundry.empty())
        return false;
    const std::string illegal = "\'()+_,-./:=?";
    for (std::string::iterator it = boundry.begin();it !=  boundry.end();++it)
        if (!isalnum(*it) && illegal.find(*it)!=std::string::npos)
            return false;
    return true;
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
    if (is_multipart)
        ProcessMultiPart();
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
    bool contunue = true;
    while (contunue)
    {
        contunue = false;
        switch (chunk_state)
        {
            case Post::READING_CHUNK_SIZE:
                ReadChunkSize();
                contunue = chunk_state != Post::READING_CHUNK_SIZE;
                break;
            case Post::READING_CHUNK_DATA:
                ReadChunkData();
                contunue = chunk_state != Post::Post::READING_CHUNK_DATA;
                break;
            case Post::READING_TRAILER_HEADERS:
                ReadTrailerHeaders();
                contunue = chunk_state != Post::READING_TRAILER_HEADERS;
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
}

void Post::ProcessMultiPart()
{
    std::string delimiter = "--" + boundry;
    bool contunue = true;
    while (contunue)
    {
        contunue = false;
        switch (multipart_state)
        {
            case READING_PREAMBLE:
            {
                size_t del = conn->buffer.find(delimiter);
                if (del != std::string::npos)
                {
                    conn->buffer.erase(0,del);
                    multipart_state = Post::READING_BOUNDARY;
                    contunue = true;
                }
            }
            break;
            case READING_BOUNDARY:
            {
                size_t close_del = conn->buffer.find(delimiter + "--");
                if (close_del !=  std::string::npos)
                {
                    conn->buffer.erase(close_del + (delimiter + "--").length() - 1);
                    multipart_state = Post::MULTIPART_COMPLETE;
                    contunue = true;
                }
                size_t CRLF = conn->buffer.find("\r\n");
                if(CRLF != std::string::npos)
                {
                    if (conn->buffer.substr(0,CRLF) != delimiter)
                    {
                        multipart_state = Post::MULTIPART_ERROR;
                        contunue = true;
                    }
                    conn->buffer.erase(0,CRLF + 2);
                    multipart_state = Post::READING_PART_HEADERS;
                    contunue = true;
                }
            }
            break;
            case READING_PART_HEADERS:
            {
                // here should be a header size  limit check
                size_t CRLFCRLF = conn->buffer.find("\r\n\r\n");
                if (CRLFCRLF != std::string::npos)
                {
                    parts.push_back(MultiPart());
                    if(!parts.back().ProcessMultiPartHeaders(conn->buffer.substr(0,CRLFCRLF))) 
                    {
                        multipart_state = Post::MULTIPART_ERROR;
                        contunue = true;
                    }
                    conn->buffer.erase(0 , CRLFCRLF + 4);
                    multipart_state = Post::READING_PART_DATA;
                    contunue = true;
                }
            }
                break;
            case READING_PART_DATA:
            {
                size_t del =  conn->buffer.find(delimiter);
                if (del !=  std::string::npos)
                {
                    // process sub-data
                    conn->buffer.erase(0,del + delimiter.length() - 1);
                    multipart_state =  Post::READING_BOUNDARY;
                    contunue = true;
                }
                //process data
                conn->buffer.clear();
            }
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
}
void Post::ProcessContentLength()
{
    size_t bytes_to_read = std::min(conn->buffer.size(),content_length - content_bytes_read);
    if (is_multipart)
        ProcessMultiPart(); 
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
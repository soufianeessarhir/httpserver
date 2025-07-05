#include "Post.hpp"
#include "MultiPart.hpp"

const std::map<std::string, std::string> Post::mime_ext = Post::createMimeExtMap();

Post::Post(Connection *conn , TransferType type):conn(conn)
{
    if (type ==  CONTENT_LENGTH)
    {
        content_length = conn->request->GetContentLenght();
        content_bytes_read = 0;
        std::string media_type = conn->request->GetHeader("content-type");
        size_t semi_colon = media_type.find(';');
        if (semi_colon != std::string::npos)
        {
            media_type = media_type.substr(0,semi_colon);
        }
        std::map<std::string,std::string>::const_iterator it = mime_ext.find(media_type);
        if (it == mime_ext.end())
        {
            //unsupporeted media type error
            transfer_type = Post::ERROR;
            return;
        }
        GenerateUploadfile(it->second);
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

void Post::WriteDataToFile(size_t size)
{
    if (!is_multipart)
    {
        return ;
    }
    else
    {
       output_file.write(conn->buffer.data(),size);
    }
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

void Post::GenerateUploadfile(const std::string &ext)
{
    struct timeval tm;
    std::stringstream oss; 
    gettimeofday(&tm,NULL);
    oss << tm.tv_sec << &tm << tm.tv_usec << &oss<<ext;
    filename = conn->location->upload_store + oss.str();
	output_file.open(filename.c_str());
   
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
				filename.clear();
                headers.clear();
                size_t CRLFCRLF = conn->buffer.find("\r\n\r\n");
                if (CRLFCRLF != std::string::npos)
                {
                    if(!ProcessMultiPartHeaders(conn->buffer.substr(0,CRLFCRLF))) 
                    {
                        multipart_state = Post::MULTIPART_ERROR;
                        contunue = true;
                        break;
                    }
                    if (headers.find("content-disposition") == headers.end())
                    {
                        multipart_state = Post::MULTIPART_ERROR;
                        contunue = true;
                        break;
                    }
                    filename = conn->location->upload_store;
                    if (!ConfigureMultipart())
                    {
                        multipart_state = Post::MULTIPART_ERROR;
                        contunue = true;
                        break;
                    }
                    if (is_file_upload)
                        parts.push_back(MultiPart(filename));
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
                    WriteDataToFile(del);
                    conn->buffer.erase(0,del + delimiter.length() - 1);
                    multipart_state =  Post::READING_BOUNDARY;
                    contunue = true;
                }
                WriteDataToFile(conn->buffer.size());
                conn->buffer.clear();
            }
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
	{
        ProcessMultiPart();
	}
	WriteDataToFile(bytes_to_read); 
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



bool Post::ProcessMultiPartHeaders(std::string data)
{
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss,line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (headers.size() == 0)
        {
            if (line.empty() || Request::OnlySpaces(line))
            {
                return false;
            }
        }
        if (line.empty())
        {
            return true;
        }
        size_t del = line.find(':');
        if(del ==  std::string::npos)
        {
            return false;
        }
        std::string name = line.substr(0,del);
        if (name.empty() || Request::Haswhitespace(name)) //[sessarhi] maybe i need to check for emply fileds | values
        {

            return false;
        }
        std::string value = line.substr(del + 1);
        Request::trim(value);
        Request::ToCanonical(name);
        headers[name] = value;
    }
    

    return true;
}

bool Post::ConfigureMultipart()
{
    std::string content_type = headers["content-disposition"];
    size_t name = content_type.find("name");
    std::string tmp;
    if (name == std::string::npos)
    {
        return false;
    }
    size_t fname = content_type.find("tmp=\"");
    if (fname != std::string::npos)
    {
        size_t next_q = content_type.find(fname,'"');
        if (next_q == std::string::npos)
            return false;
        tmp =  content_type.substr(fname,next_q - fname);
        //need to prarse the filename
        filename += tmp;
    }
    if (tmp.empty())
        is_file_upload = false;
    else
        is_file_upload = true;
    return true;
}
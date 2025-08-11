#include "Post.hpp"
#include "MultiPart.hpp"

const std::map<std::string, std::string> Post::mime_ext = Post::createMimeExtMap();

Post::Post(Connection *conn , TransferType type):transfer_type(type)
,conn(conn),is_multipart(false),is_initial_del(false)
{
    std::string content_type = conn->request->GetHeader("content-type");
    if (type ==  CONTENT_LENGTH)
    {
        content_length = conn->request->GetContentLenght();
        content_bytes_read = 0;
        if (content_type.find("multipart/form-data") != std::string::npos && !conn->UseCgi)
        {
            is_multipart = true;
            multipart_state= Post::READING_PREAMBLE;
            if (!ExtractAndValidateBoundry())
                transfer_type = Post::ERROR;
            return;
        }
        std::string media_type;
        size_t semi_colon = content_type.find(';');
        if (semi_colon != std::string::npos)
            media_type = content_type.substr(0,semi_colon);
        else 
            media_type = content_type;
        if (media_type.empty() && conn->UseCgi)
            media_type = ".bin";
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
        if (content_type.find("multipart/form-data") != std::string::npos)
        {
            is_multipart = true;
            multipart_state = Post::READING_PREAMBLE;
            if (!ExtractAndValidateBoundry())
                transfer_type = Post::ERROR;
        }
         std::string media_type;
        size_t semi_colon = content_type.find(';');
        if (semi_colon != std::string::npos)
            media_type = content_type.substr(0,semi_colon);
        else 
            media_type = content_type;
        std::map<std::string,std::string>::const_iterator it = mime_ext.find(media_type);
        if (it == mime_ext.end())
        {
            //unsupporeted media type error
            transfer_type = Post::ERROR;
            return;
        }
        GenerateUploadfile(it->second);
        chunk_state = READING_CHUNK_SIZE;
        chunk_bytes_read = 0;
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
    size_t begin = BNDR + 8;
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
            boundry = origin.substr(begin,q - begin);
    }
    else {
        for (end=begin;end < ct.length()&&
        ct[end] != ' ' && ct[end] != '\t' && 
        ct[end] != ';' && ct[end] != '\r' && ct[end] != '\n';++end );
        if (end > ct.length())
            return false;
        boundry = origin.substr(begin, end - begin);
    }
    if (boundry.empty())
        return false;
    const std::string illegal = "\'()+_,-./:=?";
    for (std::string::iterator it = boundry.begin();it !=  boundry.end();++it)
        if (!isalnum(*it) && illegal.find(*it) == std::string::npos)
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
    char *endp = NULL;
    current_chunk_size = std::strtol(size_str.c_str(),&endp,16);
    if (errno == ERANGE || !endp /*my need to check for large size*/)
    {
        conn->response = new Response(400,Error);
        chunk_state = Post::CHUNK_ERROR;
        return;
    }
    if (current_chunk_size == 0 && !size_str.empty())
        chunk_state = Post::READING_TRAILER_HEADERS;
    else
        chunk_state = Post::READING_CHUNK_DATA;
}

void Post::ReadChunkData()
{
    if (is_multipart)
    {
        size_t available_in_chunk = current_chunk_size - chunk_bytes_read;
        size_t buffer_size = conn->buffer.size();
        if (buffer_size > available_in_chunk)
        {
            std::string temp_buffer = conn->buffer.substr(0, available_in_chunk);
            std::string remaining_buffer = conn->buffer.substr(available_in_chunk);
            conn->buffer = temp_buffer;
            ProcessMultiPart();
            size_t consumed = temp_buffer.size() - conn->buffer.size();
            chunk_bytes_read += consumed;
            conn->buffer += remaining_buffer;
        }
        else
        {
            size_t initial_size = conn->buffer.size();
            ProcessMultiPart();
            size_t consumed = initial_size - conn->buffer.size();
            chunk_bytes_read += consumed;
        }
    }
    else 
    {
        size_t size_to_read = std::min(conn->buffer.size(),current_chunk_size - chunk_bytes_read);
        WriteDataToFile(size_to_read);
        conn->buffer.erase(0,size_to_read);
        chunk_bytes_read += size_to_read;
    }
    if (current_chunk_size <= chunk_bytes_read)
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
            chunk_state = Post::CHUNK_ERROR;
    }
}

void Post::ReadTrailerHeaders()
{
    // should be protected for size limits
    if (output_file.is_open())
        output_file.close();
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
    std::string delimiter = "\r\n--" + boundry;
    bool contunue = true;
    while (contunue)
    {
        contunue = false;
        switch (multipart_state)
        {
            case READING_PREAMBLE:
            {
                delimiter = "--" + boundry;
                size_t del = conn->buffer.find(delimiter);
                if (del != std::string::npos)
                {
                    conn->buffer.erase(0,del);
                    multipart_state = Post::READING_BOUNDARY;
                    is_initial_del = true;
                    contunue = true;
                }
                break;
            }
            case READING_BOUNDARY:
            {
                if (is_initial_del)
                {
                    delimiter = "--" + boundry;
                    is_initial_del = false;
                }
                 size_t  CRLF = conn->buffer.find(delimiter + "\r\n");
                if(CRLF != std::string::npos)
                {
                    conn->buffer.erase(0,(delimiter + "\r\n").size());
                    multipart_state = Post::READING_PART_HEADERS;
                    contunue = true;
                    break;
                }
                size_t close_del = conn->buffer.find(delimiter + "--");
                if (close_del !=  std::string::npos)
                {
                    conn->buffer.erase(0,close_del + delimiter.length() + 2);
                    multipart_state = Post::MULTIPART_COMPLETE;
                    contunue = true;
                    break;
                }
                if (conn->buffer.size() > (delimiter + "\r\n").size())
                {
                    multipart_state = Post::MULTIPART_ERROR;
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
                    {
                        if (output_file.is_open())
                            output_file.close();
                        output_file.open(filename.c_str(),std::ios::out | std::ios::app);
                        parts.push_back(MultiPart(filename));
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
                    if (is_file_upload)
                        WriteDataToFile(del);
                    conn->buffer.erase(0,del);
                    multipart_state =  Post::READING_BOUNDARY;
                    contunue = true;
                    break;
                }
                if (is_file_upload)
                    WriteDataToFile(conn->buffer.size());
                conn->buffer.clear();
                break;
            }
            case MULTIPART_COMPLETE:
                conn->state = Connection::SENDING_RESPONSE;
                output_file.close();
            break;
            case MULTIPART_ERROR:
                conn->state = Connection::SENDING_RESPONSE;
            break;
        }
    }
}
void Post::ProcessContentLength()
{
    size_t bytes_to_read = std::min(conn->buffer.size(),content_length - content_bytes_read + 1);
    if (is_multipart)
        ProcessMultiPart();
    else
    {
        WriteDataToFile(bytes_to_read); 
        conn->buffer.erase(0,bytes_to_read);
        content_bytes_read += bytes_to_read;
    }
    if (content_bytes_read >= content_length)
    {
        output_file.close();
        conn->state = Connection::SENDING_RESPONSE;
    }
}




bool Post::ProcessMultiPartHeaders(std::string data)
{
    std::istringstream iss(data);
    std::string line;
    size_t del;
    std::string name;
    std::string value;
    while (std::getline(iss,line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (headers.size() == 0)
            if (line.empty() || Request::OnlySpaces(line))
                return false;
        if (line.empty())
            return true;
        del = line.find(':');
        if(del ==  std::string::npos)
            return false;
        name = line.substr(0,del);
        if (name.empty() || Request::Haswhitespace(name)) //[sessarhi] maybe i need to check for emply fileds | values
            return false;
        value = line.substr(del + 1);
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
    size_t fname;
    if (name == std::string::npos)
        return false;
    fname = content_type.find("filename=\"");
    if (fname != std::string::npos)
    {
        fname += 10;
        size_t next_q = content_type.find('"' ,fname);
        if (next_q == std::string::npos)
            return false;
        tmp =  content_type.substr(fname,next_q - fname);
        if (!CheckFileName(tmp))
            return false;
        filename += tmp;
    }
    if (tmp.empty())
        is_file_upload = false;
    else
        is_file_upload = true;
    return true;
}

bool Post::CheckFileName(std::string &filename)
{
    const std::string ilegal="()<>@,;:\\\"/[]?=";
    for (size_t i = 0; i < ilegal.size();++i)
        if (filename.find(ilegal[i]) != std::string::npos)
            return false;
    return true;
}
void Post::WriteDataToFile(size_t size)
{

    output_file.write(conn->buffer.data(),size);
}

void Post::GenerateUploadfile(const std::string &ext)
{
    struct timeval tm;
    std::stringstream oss; 
    gettimeofday(&tm,NULL);
    oss << tm.tv_sec << &tm << tm.tv_usec << &oss<<ext;
    std::cout<<filename<<std::endl;
    if (conn->UseCgi)
    {
        filename = "/tmp/" + oss.str();
        conn->CgiObj->InFile = filename;
    }
    else
        filename = conn->location->upload_store + oss.str();
    output_file.open(filename.c_str(),std::ios::app |  std::ios::out);
    // output_file.open(filename.c_str(),std::ios::out | std::ios::app);
   
}
Post::~Post()
{
    if (output_file.is_open())
        output_file.close();
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 20:30:27 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/17 22:07:25 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Post.hpp"

const std::map<std::string, std::string> &         Post::createMimeExtMap()
{
    static std::map<std::string, std::string> mime_ext;
    // text
    mime_ext["text/plain"] = ".txt";
    mime_ext["text/html"] = ".html";
    mime_ext["text/css"] = ".css";
    mime_ext["text/javascript"] = ".js";
    mime_ext["text/csv"] = ".csv";
    mime_ext["text/xml"] = ".xml";
    
    // Application 
    mime_ext["application/json"] = ".json";
    mime_ext["application/pdf"] = ".pdf";
    mime_ext["application/zip"] = ".zip";
    mime_ext["application/x-tar"] = ".tar";
    mime_ext["application/x-gzip"] = ".gz";
    mime_ext["application/xml"] = ".xml"; 
    mime_ext["application/octet-stream"] = ".bin";
    mime_ext["application/x-iso9660-image"] = ".iso";
    mime_ext["application/x-www-form-urlencoded"] = ".txt"; 
    
    // Image 
    mime_ext["image/jpeg"] = ".jpg";
    mime_ext["image/png"] = ".png";
    mime_ext["image/gif"] = ".gif";
    mime_ext["image/svg+xml"] = ".svg";
    mime_ext["image/webp"] = ".webp";
    mime_ext["image/bmp"] = ".bmp";
    mime_ext["image/x-icon"] = ".ico";
    
    // Audio 
    mime_ext["audio/mpeg"] = ".mp3";
    mime_ext["audio/ogg"] = ".ogg";
    mime_ext["audio/wav"] = ".wav";
    
    // Video 
    mime_ext["video/mp4"] = ".mp4";
    mime_ext["video/ogg"] = ".ogv";
    mime_ext["video/webm"] = ".webm";
    mime_ext["video/x-msvideo"] = ".avi";
    
    // Font 
    mime_ext["font/woff"] = ".woff";
    mime_ext["font/woff2"] = ".woff2";
    mime_ext["font/ttf"] = ".ttf";
    mime_ext["font/otf"] = ".otf";
    return mime_ext;
}

const           std::map<std::string, std::string>& Post::mime_ext = Post::createMimeExtMap();

Post::Post(Connection *conn , TransferType type):transfer_type(type),conn(conn),is_multipart(false)
{
    std::string content_type = conn->request->GetHeader("content-type");
    max_body_size = conn->location->max_body_size;
    if (type ==  CONTENT_LENGTH)
    {
        content_length = conn->request->GetContentLenght();
        content_bytes_read = 0;
        ChecKMultiPart(content_type);
        if (is_multipart)
            return;
        ProcessMediaType(content_type);
    }
    else
    {
        chunk_state = READING_CHUNK_SIZE;
        current_chunk_size = 0;
        chunk_bytes_read = 0;
        ChecKMultiPart(content_type);
        if (is_multipart)
            return;
        ProcessMediaType(content_type);
    }
}

void  Post::ChecKMultiPart(std::string &content_type)
{
    if (content_type.find("multipart/form-data") != std::string::npos && !conn->UseCgi)
    {
        is_multipart = true;
        multipart_state= Post::READING_PREAMBLE;
        if (!ExtractAndValidateBoundry())
            transfer_type = Post::ERROR;
    }
}
void Post::ProcessMediaType(std::string &content_type)
{
    std::string media_type;
    size_t semi_colon = content_type.find(';');
    if (semi_colon != std::string::npos)
        media_type = content_type.substr(0,semi_colon);
    else 
        media_type = content_type;
    if (conn->UseCgi)
        media_type = "application/octet-stream";
    std::map<std::string,std::string>::const_iterator it = mime_ext.find(media_type);
    if (it == mime_ext.end())
    {
        conn->response = new Response(415,Error);
        conn->state =  Connection::SENDING_RESPONSE;
        transfer_type = Post::ERROR;
        return;
    }
    GenerateUploadfile(it->second);
}

bool           Post::ExtractAndValidateBoundry()
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
        size_t q = ct.find('"', begin);
        if (q != std::string::npos)
            boundry = origin.substr(begin,q - begin);
    }
    else 
    {
        for (end=begin;end < ct.length()&&
        ct[end] != ' ' && ct[end] != '\t' && 
        ct[end] != ';' && ct[end] != '\r' && ct[end] != '\n';++end );
        if (end > ct.length())
            return false;
        boundry = origin.substr(begin, end - begin);
    }
    if (boundry.empty() || boundry.length() > 70)
        return false;
    const std::string illegal = "\'()+_,-./:=?";
    for (std::string::iterator it = boundry.begin();it !=  boundry.end();++it)
        if (!isalnum(*it) && illegal.find(*it) == std::string::npos)
            return false;
    initial_boundry = "--" + boundry + "\r\n";
    subsequent_boundry  = "\r\n--" + boundry + "\r\n";
    close_boundry = "\r\n--" + boundry + "--\r\n";
    delimiter = "\r\n--" + boundry;
    return true;
}


void Post::ProcessContentLength()
{
    size_t bytes_to_read = std::min(conn->buffer.size(), content_length - content_bytes_read);
    if (is_multipart)
    {
        part_buffer.append(conn->buffer.data(),bytes_to_read);
        conn->buffer.erase(0,bytes_to_read);
        ProcessMultiPart();
    }
    else
        WriteDataToFile(bytes_to_read); 
        conn->buffer.erase(0, bytes_to_read);
    content_bytes_read += bytes_to_read;
    if (content_bytes_read >= content_length)
    {
        if (output_file.is_open())
            output_file.close();
        conn->state = Connection::SENDING_RESPONSE;
    }
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
    if (conn->CgiObj)
    {
        filename = "/tmp/" + oss.str();
        conn->CgiObj->InFile = filename;
    }
    else
        filename = conn->location->upload_store + oss.str();
    output_file.open(filename.c_str(),std::ios::out | std::ios::trunc | std::ios::binary);
    if (output_file.bad())
    {
        transfer_type = Post::ERROR;
        conn->response = new Response(500,Error);
        conn->state = Connection::SENDING_RESPONSE;
    }
   
}
Post::~Post()
{
    if (output_file.is_open())
        output_file.close();
}
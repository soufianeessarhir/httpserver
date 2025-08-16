/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 20:31:37 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/16 10:06:42 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Post.hpp"

void Post::ReadPreamble()
{
    size_t del = conn->buffer.find(initial_boundry);
    if (del != std::string::npos)
    {
        conn->buffer.erase(0,del + initial_boundry.length());
        multipart_state = Post::READING_PART_HEADERS;
        contunue = true;
    }
}
void Post::ReadBoundry()
{
    size_t  CRLF = conn->buffer.find(subsequent_boundry);
    if(CRLF != std::string::npos)
    {
        conn->buffer.erase(0,subsequent_boundry.length());
        multipart_state = Post::READING_PART_HEADERS;
        contunue = true;
        return;
    }
    size_t close_del = conn->buffer.find(close_boundry);
    if (close_del !=  std::string::npos)
    {
        conn->buffer.erase(0, close_boundry.length());
        multipart_state = Post::MULTIPART_COMPLETE;
        contunue = true;
        return;
    }
    if (conn->buffer.length() > subsequent_boundry.length())
    {
        multipart_state = Post::MULTIPART_ERROR;
        contunue = true;
    }
}
void Post::ReadPartHeaders()
{
    filename.clear();
    headers.clear();
    size_t CRLFCRLF = conn->buffer.find("\r\n\r\n");
    if (CRLFCRLF != std::string::npos)
    {
        if(!ProcessMultiPartHeaders(conn->buffer.substr(0,CRLFCRLF))) 
        {
            multipart_state = Post::MULTIPART_ERROR;
            contunue = true;
            return;
        }
        if (headers.find("content-disposition") == headers.end())
        {
            multipart_state = Post::MULTIPART_ERROR;
            contunue = true;
            return;
        }
        filename = conn->location->upload_store;
        if (!ConfigureMultipart())
        {
            multipart_state = Post::MULTIPART_ERROR;
            contunue = true;
            return;
        }
        if (is_file_upload)
        {
            if (output_file.is_open())
                output_file.close();
            output_file.open(filename.c_str(),std::ios::out | std::ios::app | std::ios::binary);
        }
        conn->buffer.erase(0 , CRLFCRLF + 4);
        multipart_state = Post::READING_PART_DATA;
        contunue = true;
    }
}
void Post::ReadPartData()
 {
    size_t pos = conn->buffer.find(delimiter);
    if (pos != std::string::npos) {
        if (is_file_upload)
            output_file.write(conn->buffer.data(), pos);
        conn->buffer.erase(0, pos);
        multipart_state = Post::READING_BOUNDARY;
        contunue = true;
        return;
    }
    size_t k = delimiter.size();
    if (conn->buffer.size() >= k) {
        size_t safe =  conn->buffer.size() - (k - 1);
        if (is_file_upload && safe > 0)
            output_file.write( conn->buffer.data(),safe);
        conn->buffer.erase(0, safe);
    }
}
void Post::ProcessMultiPart()
{
    contunue = true;
    while (contunue)
    {
        contunue = false;
        switch (multipart_state)
        {
            case READING_PREAMBLE:
                ReadPreamble();
                break;
            case READING_BOUNDARY:
                ReadBoundry();
                break;
            case READING_PART_HEADERS:
				ReadPartHeaders();
                break;
            case READING_PART_DATA: 
                ReadPartData();
                break;
            case MULTIPART_COMPLETE:
                if(output_file.is_open())
                    output_file.close();
                break;
            case MULTIPART_ERROR:
                conn->response = new Response(400,Error);
                conn->state = Connection::SENDING_RESPONSE;
                if(output_file.is_open())
                    output_file.close();
                break;
        }
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
        if (name.empty() || Request::Haswhitespace(name))
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 20:32:17 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/24 11:00:36 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"


void Post::ReadChunkSize()
{
    size_t CRLF = conn->buffer.find("\r\n");
    if (CRLF == std::string::npos)
        return;
    std::string size_str = conn->buffer.substr(0, CRLF);
    conn->buffer.erase(0, CRLF + 2);
    size_t hex_end = 0;
    while (hex_end < size_str.size() && std::isxdigit(static_cast<unsigned char>(size_str[hex_end])))
        ++hex_end;
    errno = 0;
    char *endp = NULL;
    long val = std::strtol(size_str.c_str(), &endp, 16);
    if (errno == ERANGE || endp == size_str.c_str() || val < 0) {
        conn->response = new Response(400, Error);
        chunk_state = Post::CHUNK_ERROR;
        return;
    }
    current_chunk_size = static_cast<size_t>(val);
    if (current_chunk_size == 0)
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
            std::string remaining_buffer = conn->buffer.substr(available_in_chunk);
            conn->buffer = part_buffer +  conn->buffer.substr(0, available_in_chunk);;
            ProcessMultiPart();
            part_buffer =  conn->buffer;
            conn->buffer = remaining_buffer;
            chunk_bytes_read += available_in_chunk;
        }
        else
        {
            conn->buffer = part_buffer + conn->buffer;
            ProcessMultiPart();
            part_buffer = conn->buffer;
            conn->buffer.clear();
            chunk_bytes_read += buffer_size;
            
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
        if (CRLF == 0)
        {
            conn->buffer.erase(0, 2);
            chunk_state = Post::READING_CHUNK_SIZE;
            current_chunk_size = 0;
            chunk_bytes_read = 0;
        }
        else if (CRLF != std::string::npos)
            chunk_state = Post::CHUNK_ERROR;
    }
}

void           Post::ReadTrailerHeaders()
{
    if (output_file.is_open())
        output_file.close();
    size_t CRLF = conn->buffer.find("\r\n\r\n");
    if (CRLF != std::string::npos)
    {
        conn->buffer.erase(0,CRLF + 4);
        chunk_state = Post::CHUNK_COMPLETE;
    }
    else if ( conn->buffer.find("\r\n") == 0)
    {
        conn->buffer.erase(0,2);
        chunk_state = Post::CHUNK_COMPLETE;
    }
}

void Post::ProcessChunck()
{
    contunue = true;
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
            contunue = chunk_state != Post::READING_CHUNK_DATA;
            break;
            case Post::READING_TRAILER_HEADERS:
            ReadTrailerHeaders();
            contunue = chunk_state != Post::READING_TRAILER_HEADERS;
            break;
            case Post::CHUNK_COMPLETE:
            conn->state = Connection::SENDING_RESPONSE;
            break;
            case Post::CHUNK_ERROR:
            conn->state = Connection::SENDING_RESPONSE; 
            break;
        }
    }
}
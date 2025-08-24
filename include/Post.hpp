/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 20:31:49 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/24 11:01:33 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef POST_HPP
#define POST_HPP

#include <fstream>
#include "Connection.hpp"
#include "sys/time.h"
#include <sstream>


class Connection;
class Post
{
public:
    enum  TransferType 
    {
        CONTENT_LENGTH,
        CHUNKED,
        ERROR
    };
    enum  ChunkState 
    {
        READING_CHUNK_SIZE,
        READING_CHUNK_DATA,
        READING_TRAILER_HEADERS,
        CHUNK_COMPLETE,
        CHUNK_ERROR          
    };
    enum MultiPaertState
    {
        READING_PREAMBLE,       
        READING_BOUNDARY,       
        READING_PART_HEADERS,   
        READING_PART_DATA,
        MULTIPART_COMPLETE,
        MULTIPART_ERROR   
    };
    
    Post(Connection * , TransferType);
    ~Post();
    void ProcessChunck();
    void ProcessContentLength();
    TransferType transfer_type;

private:
	std::map<std::string, std::string> headers;
    ChunkState chunk_state;
    Connection *conn;
    size_t current_chunk_size;
    size_t chunk_bytes_read;
    size_t content_length;
    size_t content_bytes_read;
    std::ofstream output_file;
    size_t max_body_size;
    bool is_multipart;
    bool contunue;
    std::string boundry;
    std::string initial_boundry;
    std::string subsequent_boundry;
    std::string close_boundry;
    std::string delimiter;
	std::string filename;
	bool is_file_upload;
    std::string part_buffer;
    MultiPaertState multipart_state;
    static const std::map<std::string, std::string>& mime_ext;


    void ReadChunkSize();
    void ReadChunkData();
    void ReadTrailerHeaders();
    void ProcessMultiPart();
    bool ExtractAndValidateBoundry();
    void ChecKMultiPart(std::string &content_type);
    void ProcessMediaType(std::string &content_type);
    void GenerateUploadfile(const std::string &ext);
	bool ProcessMultiPartHeaders(std::string data);
    bool ConfigureMultipart();
    void WriteDataToFile(size_t size);
    bool CheckFileName(std::string &);
    static const std::map<std::string, std::string>& createMimeExtMap();
    void ReadBoundry();
    void ReadPartHeaders();
    void ReadPartData();
    void ReadPreamble();
};

#endif
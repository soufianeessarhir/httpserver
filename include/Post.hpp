#ifndef POST_HPP
#define POST_HPP

#include <fstream>
#include "Connection.hpp"
#include "MultiPart.hpp"
#include "sys/time.h"
#include <sstream>
class Connection;
struct  MultiPart;
class Post
{
public:
    enum  TransferType {
    CONTENT_LENGTH,
    CHUNKED,
    ERROR
    };
    enum  ChunkState {
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
    bool is_initial_del;
    std::string boundry;
	std::string filename;
	bool is_file_upload;
    std::vector<MultiPart> parts;
    MultiPaertState multipart_state;


private:
    void ReadChunkSize();
    void ReadChunkData();
    void ReadTrailerHeaders();
    void ProcessMultiPart();
    bool ExtractAndValidateBoundry();
    void GenerateUploadfile(const std::string &ext);
	bool ProcessMultiPartHeaders(std::string data);
    bool ConfigureMultipart();
    void WriteDataToFile(size_t size);
    bool CheckFileName(std::string &);

private:
      static std::map<std::string, std::string> createMimeExtMap() {
        std::map<std::string, std::string> mime_ext;
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
    static const std::map<std::string, std::string> mime_ext;
    
};

#endif
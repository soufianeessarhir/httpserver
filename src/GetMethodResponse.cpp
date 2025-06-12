/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 00:36:32 by eaboudi           #+#    #+#             */
/*   Updated: 2025/06/11 23:13:24 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetMethodResponse.hpp"
#include <sstream>

std::map<std::string, std::string> CreateMimeTypes()
{
    std::map<std::string, std::string> mimeTypes;
    mimeTypes["html"] = "text/html";
    mimeTypes["htm"]  = "text/html";
    mimeTypes["css"]  = "text/css";
    mimeTypes["js"]   = "application/javascript";
    mimeTypes["json"] = "application/json";
    mimeTypes["txt"]  = "text/plain";
    mimeTypes["xml"]  = "application/xml";

    mimeTypes["jpg"]  = "image/jpeg";
    mimeTypes["jpeg"] = "image/jpeg";
    mimeTypes["png"]  = "image/png";
    mimeTypes["gif"]  = "image/gif";
    mimeTypes["bmp"]  = "image/bmp";
    mimeTypes["ico"]  = "image/x-icon";
    mimeTypes["svg"]  = "image/svg+xml";

    mimeTypes["pdf"]  = "application/pdf";
    mimeTypes["zip"]  = "application/zip";
    mimeTypes["tar"]  = "application/x-tar";
    mimeTypes["gz"]   = "application/gzip";
    mimeTypes["rar"]  = "application/vnd.rar";

    mimeTypes["mp3"]  = "audio/mpeg";
    mimeTypes["wav"]  = "audio/wav";
    mimeTypes["ogg"]  = "audio/ogg";

    mimeTypes["mp4"]  = "video/mp4";
    mimeTypes["webm"] = "video/webm";
    mimeTypes["avi"]  = "video/x-msvideo";
    mimeTypes["mpeg"] = "video/mpeg";
    mimeTypes["mov"]  = "video/quicktime";

    return mimeTypes;
}

const std::map<std::string, std::string> GetMethodResponse::MimeTypes = CreateMimeTypes();

void    GetMethodResponse::SetContentType()
{
    std::string Extension = FilePath.substr(FilePath.find_last_of('.') + 1);
    std::map<std::string, std::string>::const_iterator it = MimeTypes.find(Extension);
    if (it != MimeTypes.end())
    {
        ContentType = it->second;
        IsBinaryFile = (ContentType.find("text/") != 0 && 
                   ContentType != "application/javascript" &&
                   ContentType != "application/json" &&
                   ContentType != "application/xml" &&
                   ContentType != "image/svg+xml");
    }
    else
    {
        ContentType = "application/octet-stream"; // Default type for unknown files
        IsBinaryFile = true;
    }
}

GetMethodResponse::GetMethodResponse(int statusCode, std::string filePath)
    : BaseResponse(statusCode), FilePath(filePath), IsBinaryFile(false)
{
    SetContentType();
    SetBody();
    SetHeaders();
    SetStatusLine();
}

void    GetMethodResponse::SetBody()
{
    // if [sofyan] didn't check all of the cases of file reading. I'll do it here
    std::ifstream InFile;
    if (IsBinaryFile)
        InFile.open(FilePath.c_str(), std::ios::binary);
    else
        InFile.open(FilePath.c_str());
    if (!InFile)
    {
        StatusCode = 404;
        Body.clear();
        return;
    }
    InFile.seekg(0, std::ios::end);
    std::streampos fileSize = InFile.tellg();
    if (fileSize == std::streampos(-1))
    {
        // Handle error - file size couldn't be determined
        StatusCode = 500;
        return;
    }
    InFile.seekg(0, std::ios::beg);
    Body.assign(std::istreambuf_iterator<char>(InFile),
            std::istreambuf_iterator<char>());
    InFile.close();
    ContentLength = Body.size();
    StatusCode = 200;
}

void    GetMethodResponse::SetHeaders()
{
    std::ostringstream oss;
    oss << ContentLength;
    Headers["Content-Length"] = oss.str() + "\r\n";
    // std::to_string is not used for compatibility with older C++ standards
    std::ostringstream cl_oss;
    cl_oss << ContentLength;
    Headers["Content-Length"] = cl_oss.str() + "\r\n";
    Headers["Content-Type"] = ContentType + "\r\n";
    // Headers["Connection"] = "close"; add it if we need it
}

GetMethodResponse::~GetMethodResponse()
{
    
}


/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 00:36:32 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/10 15:35:15 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetMethodResponse.hpp"
#include <sstream>
#include <sys/stat.h>

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
const std::map<int, std::string> GetMethodResponse::ErrorPhrase = createErrorPhrase();

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
    : StatusCode(statusCode), FilePath(filePath), IsBinaryFile(false)
{
    SetContentType();
    SetBody();
    SetHeaders();
    SetStatusLine();
}

void GetMethodResponse::SetStatusLine()
{
    std::stringstream BuildStatusLine;
    BuildStatusLine << "HTTP/1.1 ";
    std::map<int, std::string>::const_iterator it = ErrorPhrase.find(StatusCode);
    if (it != ErrorPhrase.end())
        BuildStatusLine << StatusCode << " " << it->second << "\r\n";
    else
        BuildStatusLine << StatusCode << " OK\r\n";
    StatusLine = BuildStatusLine.str();
}

void    GetMethodResponse::SetBody()
{
    struct stat FileState;
    if (stat(FilePath.c_str(), &FileState) == -1)
    {
        StatusCode = 404;
        Body.clear();
        return;
    }
    if (!S_ISREG(FileState.st_mode))
    {
        StatusCode = 403; // Forbidden if not a regular file
        Body.clear();
        return;
    }
    
    std::ifstream InFile;
    std::ios_base::openmode FileMode(std::ios::in);
    if (IsBinaryFile)
        FileMode |= std::ios::binary;
    InFile.open(FilePath.c_str(), FileMode);
    if (!InFile.is_open())
    {
        StatusCode = 403;
        Body.clear();
        return;
    }
    ContentLength = FileState.st_size;
    Body.resize(ContentLength);
    if (ContentLength > 0)
    {
        InFile.read(&Body[0], ContentLength);
        if (!InFile)
        {
            StatusCode = 500;
            Body.clear();
            return;
        }
    }
    else
        Body.clear();
    InFile.close();
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

void GetMethodResponse::SendStatusLine(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    while (BytesSent < static_cast<ssize_t>(StatusLine.size()))
    {
        BytesWriten = send(Conn->fd, StatusLine.c_str() + BytesSent, 
                            StatusLine.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
    }
    if (BytesSent >= static_cast<ssize_t>(StatusLine.size()))
        BytesSent = 0;
}

const std::string& GetMethodResponse::GetBody() const
{
    return Body;
}
const std::string& GetMethodResponse::GetContentType() const
{
    return ContentType;
}

int GetMethodResponse::GetStatusCode() const
{
    return StatusCode;
}

void GetMethodResponse::SendHeaders(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    std::string HeadersStr;
    std::map<std::string, std::string>::const_iterator it;
    for (it = Headers.begin(); it != Headers.end(); ++it)
        HeadersStr += it->first + ": " + it->second;
    HeadersStr += "\r\n";
    while (BytesSent < static_cast<ssize_t>(HeadersStr.size()))
    {
        BytesWriten = send(Conn->fd, HeadersStr.c_str() + BytesSent, 
                            HeadersStr.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
    }
    if (BytesSent >= static_cast<ssize_t>(HeadersStr.size()))
        BytesSent = 0;
}

void GetMethodResponse::SendBody(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    while (BytesSent < static_cast<ssize_t>(Body.size()))
    {
        BytesWriten = send(Conn->fd, Body.c_str() + BytesSent, 
                            Body.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
    }
    if (BytesSent >= static_cast<ssize_t>(Body.size()))
        BytesSent = 0;
}

void    excuteGetMethod(Connection *conn)
{
    std::string Path = conn->request->GetUri();
    // if (conn->request->UseCgi == true)
    // {
    //     conn->request->CgiObj->BuildEnv();
    // }
    conn->response->GET = new GetMethodResponse(conn->request->GetStatus(), Path);
    conn->response->GET->SetStatusLine();
    conn->response->GET->SendStatusLine(conn);
    conn->response->GET->SendHeaders(conn);
	if (conn->response->GET->GetStatusCode() == 200)
    {
		conn->response->GET->SendBody(conn);
		if (conn->response->GET->GetBody().empty())
			conn->state = Connection::COMPLETE; // No body to send
	}
}


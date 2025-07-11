/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 00:36:32 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/11 10:36:24 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetMethodResponse.hpp"
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>

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
    // SetContentType();
    // SetBody();
    // SetHeaders();
    // SetStatusLine();
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


void    GetMethodResponse::SetHeaders()
{
    std::ostringstream oss;
    oss << ContentLength;
    Headers["Content-Length"] = oss.str() + "\r\n";
    Headers["Content-Type"] = ContentType + "\r\n";
    // Headers["Connection"] = "close"; add it if we need it
}

GetMethodResponse::~GetMethodResponse()
{
    
}

void GetMethodResponse::SendStatusLine(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    size_t TotalSent = 0;
    
    while (TotalSent < StatusLine.size())
    {
        BytesWriten = send(Conn->fd, StatusLine.c_str() + TotalSent, 
                          StatusLine.size() - TotalSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else if (errno == EPIPE || errno == ECONNRESET)
            {
                // Client disconnected - clean up and exit
                Conn->state = Connection::COMPLETE;
                return;
            }
            else
            {
                perror("send status line");
                Conn->state = Connection::COMPLETE;
                return;
            }
        }
        TotalSent += BytesWriten;
    }
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
    size_t TotalSent = 0;
    std::string HeadersStr;
    
    std::map<std::string, std::string>::const_iterator it;
    for (it = Headers.begin(); it != Headers.end(); ++it)
        HeadersStr += it->first + ": " + it->second;
    HeadersStr += "\r\n";
    
    while (TotalSent < HeadersStr.size())
    {
        BytesWriten = send(Conn->fd, HeadersStr.c_str() + TotalSent, 
                          HeadersStr.size() - TotalSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return;
            }
            else if (errno == EPIPE || errno == ECONNRESET)
            {
                Conn->state = Connection::COMPLETE;
                return;
            }
            else
            {
                perror("send headers");
                Conn->state = Connection::COMPLETE;
                return;
            }
        }
        TotalSent += BytesWriten;
    }
}

bool    GetMethodResponse::CheckForSending()
{
    struct stat FileState;
    if (stat(FilePath.c_str(), &FileState) == -1 || !S_ISREG(FileState.st_mode))
    {
        StatusCode = 404;
        return false;
    }
    CheckProg.FileFd = open(FilePath.c_str(), O_RDONLY);
    if (CheckProg.FileFd == -1)
    {
        StatusCode = 403;
        return false;
    }
    CheckProg.FileOffset = 0;
    CheckProg.FileSize = FileState.st_size;
    CheckProg.BuffSize = 0;
    CheckProg.BuffOffs = 0;
    ContentLength = FileState.st_size;
    return true;
}

void GetMethodResponse::SetAndSendBody(Connection* conn) 
{
    if (CheckForSending() == false)
        return ;
    SetHeaders();
    SendHeaders(conn);
    char Buff[BUFFER_SIZE];
    while (true) 
    {
        // Step 1: If buffer is empty, read more from file
        if (CheckProg.BuffOffs >= CheckProg.BuffSize)
        {
            ssize_t bytes_read = read(CheckProg.FileFd, Buff, BUFFER_SIZE);
            if (bytes_read < 0) 
            {
                perror("read");
                conn->state = Connection::COMPLETE;
                close(CheckProg.FileFd);
                return;
            }
            else if (bytes_read == 0)
            {
                // EOF reached
                conn->state = Connection::COMPLETE;
                close(CheckProg.FileFd);
                return;
            }
            CheckProg.BuffSize = bytes_read;
            CheckProg.BuffOffs = 0;
        }

        // Step 2: Send from buffer
        ssize_t bytes_sent = send(conn->fd,
                                  Buff + CheckProg.BuffOffs,
                                  CheckProg.BuffSize - CheckProg.BuffOffs,
                                  0);
        if (bytes_sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Try again later when socket is writable
                return;
            }
            // else
            // {
            //     perror("send");
            //     conn->state = Connection::COMPLETE;
            //     close(CheckProg.FileFd);
            //     return;
            // }
        }

        CheckProg.BuffOffs += bytes_sent;
    }
}

void    GetMethodResponse::SetContentLenght(int i)
{
    ContentLength = i;
}

void    excuteGetMethod(Connection *conn)
{
    std::string Path = conn->request->GetUri();
    // if (conn->request->UseCgi == true)
    // {
    //     conn->request->CgiObj->BuildEnv();
    // }
    conn->response->GET = new GetMethodResponse(conn->response->GetStatusCode(), Path);
    conn->response->GET->SetStatusLine();
    conn->response->GET->SendStatusLine(conn);
    if (conn->response->GET->GetStatusCode() != 200)
    {    
        conn->response->GET->SetContentLenght(0);
        conn->response->GET->SetHeaders();
        conn->response->GET->SendHeaders(conn);
    }
	else
    {
        // conn->response->GET->SendBody(conn);
        conn->response->GET->SetAndSendBody(conn);
		if (conn->response->GET->GetBody().empty())
			conn->state = Connection::COMPLETE; // No body to send
	}
}


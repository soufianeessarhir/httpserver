/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 00:36:32 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/12 12:04:17 by eaboudi          ###   ########.fr       */
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
    ResponseStat = SENDING_STATUSLINE;
    SetContentType();
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


void    GetMethodResponse::SetHeaders(bool CloseConn)
{
    if (!CloseConn)
    {
        std::ostringstream oss;
        oss << ContentLength;
        Headers["Content-Type"] = ContentType + "\r\n";
        Headers["Content-Length"] = oss.str() + "\r\n";
        return ;
    }
    ContentType = "text/html";
    ContentLength = 0;
    Headers["Content-Type"] = ContentType + "\r\n";
    Headers["Content-Length"] = "0\r\n";
    Headers["Connection"] = "close";
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
                          StatusLine.size() - TotalSent, MSG_NOSIGNAL);
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
                          HeadersStr.size() - TotalSent, MSG_NOSIGNAL);
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
    // Allocate buffer if not already done
    // if (!CheckProg.Buff)
    //     CheckProg.Buff = new char[BUFFER_SIZE];
    ContentLength = FileState.st_size;
    return true;
}
void GetMethodResponse::SetAndSendBody(Connection* conn) 
{
    // Make Buff a member of CheckProg so it persists between calls
    if (CheckProg.BuffOffs >= CheckProg.BuffSize)
    {
        ssize_t bytes_read = read(CheckProg.FileFd, CheckProg.Buff, BUFFER_SIZE);
        if (bytes_read < 0)
        {
            perror("read");
            conn->state = Connection::COMPLETE;
            close(CheckProg.FileFd);
            return;
        }
        else if (bytes_read == 0)
        {
            ResponseStat = SENDING_COMPLETE;
            conn->state = Connection::COMPLETE;
            close(CheckProg.FileFd);
            return;
        }
        CheckProg.BuffSize = bytes_read;
        CheckProg.BuffOffs = 0;
    }
    ssize_t bytes_sent = send(conn->fd,
                                CheckProg.Buff + CheckProg.BuffOffs,
                                CheckProg.BuffSize - CheckProg.BuffOffs,
                                MSG_NOSIGNAL);
    if (bytes_sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        else
        {
            perror("send");
            conn->state = Connection::COMPLETE;
            close(CheckProg.FileFd);
            return;
        }
    }
    CheckProg.BuffOffs += bytes_sent;
}

void    excuteGetMethod(Connection *conn)
{
    if (!conn->response->GET)
    {
        std::string Path = conn->request->GetUri();
        conn->response->GET = new GetMethodResponse(conn->response->GetStatusCode(), Path);
    }
    switch (conn->response->GET->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            conn->response->GET->CheckForSending();
            conn->response->GET->SetStatusLine();
            conn->response->GET->SendStatusLine(conn);
            if (conn->response->GET->GetStatusCode() != 200)
            {
                conn->response->GET->SetHeaders(true);
                conn->response->GET->SendHeaders(conn);
                conn->response->GET->ResponseStat = SENDING_COMPLETE;
            }
            else
                conn->response->GET->ResponseStat = SENDING_HEADERS;
            break;
        }
        case SENDING_HEADERS :
        {
            conn->response->GET->SetHeaders(false);
            conn->response->GET->SendHeaders(conn);
            conn->response->GET->ResponseStat = SENDING_BODY;
            break ;
        }
        case SENDING_BODY :
        {
            conn->response->GET->SetAndSendBody(conn);
            // If file sending is complete, update state
            if (conn->response->GET->ResponseStat == SENDING_COMPLETE)
                conn->state = Connection::COMPLETE;
            break;
        }
        case SENDING_COMPLETE :
        {
            conn->state = Connection::COMPLETE;
            break ;
        }
    }
}


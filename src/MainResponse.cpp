/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MainResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 00:36:32 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/19 08:49:59 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MainResponse.hpp"
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
class MainResponse;

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

std::map<int, std::string> CreateMapOfHtmlErrors()
{
    std::map<int, std::string> Map;

    Map[400] = "Indexes/BadRequest.html";
    Map[401] = "Indexes/Unauthorized.html";
    Map[403] = "Indexes/Forbidden.html";
    Map[404] = "Indexes/NotFound.html";
    Map[405] = "Indexes/MethodNotAllowed.html";
    Map[406] = "Indexes/NotAcceptable.html";
    Map[408] = "Indexes/RequestTimeout.html";
    Map[409] = "Indexes/Conflict.html";
    Map[410] = "Indexes/Gone.html";
    Map[411] = "Indexes/LengthRequired.html";
    Map[412] = "Indexes/PreconditionFailed.html";
    Map[413] = "Indexes/PayloadTooLarge.html";
    Map[414] = "Indexes/URITooLong.html";
    Map[415] = "Indexes/UnsupportedMediaType.html";
    Map[416] = "Indexes/RangeNotSatisfiable.html";
    Map[417] = "Indexes/ExpectationFailed.html";
    Map[418] = "Indexes/ImATeapot.html";
    Map[421] = "Indexes/MisdirectedRequest.html";
    Map[422] = "Indexes/UnprocessableEntity.html";
    Map[423] = "Indexes/Locked.html";
    Map[424] = "Indexes/FailedDependency.html";
    Map[425] = "Indexes/TooEarly.html";
    Map[426] = "Indexes/UpgradeRequired.html";
    Map[428] = "Indexes/PreconditionRequired.html";
    Map[429] = "Indexes/TooManyRequests.html";
    Map[431] = "Indexes/RequestHeaderFieldsTooLarge.html";
    Map[451] = "Indexes/UnavailableForLegalReasons.html";
    Map[500] = "Indexes/InternalServerError.html";
    Map[501] = "Indexes/NotImplemented.html";
    Map[502] = "Indexes/BadGateway.html";
    Map[503] = "Indexes/ServiceUnavailable.html";
    Map[504] = "Indexes/GatewayTimeout.html";
    Map[505] = "Indexes/HTTPVersionNotSupported.html";
    Map[506] = "Indexes/VariantAlsoNegotiates.html";
    Map[507] = "Indexes/InsufficientStorage.html";
    Map[508] = "Indexes/LoopDetected.html";
    Map[510] = "Indexes/NotExtended.html";
    Map[511] = "Indexes/NetworkAuthenticationRequired.html";
    Map[204] = "Indexes/NoContent.html";
    Map[200] = "Indexes/Success.html";

    return Map;
}


const std::map<std::string, std::string> MainResponse::MimeTypes = CreateMimeTypes();
const std::map<int, std::string> MainResponse::ErrorPhrase = createErrorPhrase();
const std::map<int, std::string> MainResponse::ErrorHtmlPath  = CreateMapOfHtmlErrors();

void    MainResponse::SetContentType(Connection *conn)
{
    std::string Extension = conn->request->GetUri().substr(conn->request->GetUri().find_last_of('.') + 1);
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
    if(StatusCode != 200)
        ContentType = "text/html";
}

MainResponse::MainResponse(int statusCode) : StatusCode(statusCode), IsBinaryFile(false)
{
    ResponseStat = SENDING_STATUSLINE;
    ContentLength = 0;
}

void MainResponse::SetStatusLine()
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


void    MainResponse::SetHeaders(bool CloseConn, Connection *conn)
{
    std::cout << ContentLength << std::endl;
    if (!CloseConn)
    {
        Headers["Connection"] = "keep-alive\r\n";
        std::string CookieContent = conn->request->GetHeader("cookie");
        if (!CookieContent.empty())
            Headers["Set-Cookie"] = CookieContent + "\r\n";
    }
    else
        Headers["Connection"] = "close\r\n";
    std::ostringstream oss;
    oss << ContentLength;
    if (conn->UseCgi == false)
    {
        Headers["Content-Length"] = oss.str() + "\r\n";
        Headers["Content-Type"] = ContentType + "\r\n";
    }
    Headers["Content-Length"] = oss.str() + "\r\n";
    Headers["Content-Type"] = "text/html\r\n";
}

MainResponse::~MainResponse()
{
    
}

void MainResponse::SendStatusLine(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    size_t TotalSent = 0;
    
    while (TotalSent < StatusLine.size())
    {
        BytesWriten = send(Conn->fd, StatusLine.c_str() + TotalSent, 
                          StatusLine.size() - TotalSent, MSG_NOSIGNAL);
        
        if (BytesWriten == 0)
        {
            Conn->state = Connection::SENDING_RESPONSE;
            return;
        }
        if (BytesWriten < 0)
        {
            perror("send status line");
            Conn->state = Connection::COMPLETE;
            return;
        }
        TotalSent += BytesWriten;
    }
}

const std::string& MainResponse::GetContentType() const
{
    return ContentType;
}

int MainResponse::GetStatusCode() const
{
    return StatusCode;
}

void MainResponse::SendHeaders(Connection *conn)
{
    ssize_t BytesWriten = 0;
    size_t TotalSent = 0;
    std::string HeadersStr;

    std::map<std::string, std::string>::const_iterator it;
    for (it = Headers.begin(); it != Headers.end(); ++it)
        HeadersStr += it->first + ": " + it->second;
    if (conn->UseCgi && (conn->request->GetMethod() == "POST" || conn->request->GetMethod() == "GET") 
        && !conn->CgiObj->CgiHeaders.empty())
    {
       std::map<std::string, std::string>::const_iterator it(conn->CgiObj->CgiHeaders.begin());
       for(;it != Headers.end(); ++it)
            HeadersStr += it->first + ": " + it->second;
    }
    HeadersStr += "\r\n";
    
    while (TotalSent < HeadersStr.size())
    {
        BytesWriten = send(conn->fd, HeadersStr.c_str() + TotalSent, 
                          HeadersStr.size() - TotalSent, MSG_NOSIGNAL);
        
        if (BytesWriten == 0)
        {
            conn->state = Connection::SENDING_RESPONSE;
            return ;
        }
        if (BytesWriten < 0)
        {
            perror("send headers");
            conn->state = Connection::COMPLETE;
            return;
        }
        TotalSent += BytesWriten;
    }
}

bool    MainResponse::CheckForSending(Connection *conn)
{
    struct stat FileState;
    stat(conn->request->GetUri().c_str(), &FileState);
    CheckProg.FileFd = open(conn->request->GetUri().c_str(), O_RDONLY);
    CheckProg.FileOffset = 0;
    CheckProg.FileSize = FileState.st_size;
    CheckProg.BuffSize = 0;
    CheckProg.BuffOffs = 0;
    ContentLength = FileState.st_size;
    return true;
}

void MainResponse::SetAndSendBody(Connection* conn) 
{
    if (CheckProg.BuffOffs >= CheckProg.BuffSize)
    {
        ssize_t bytes_read = read(CheckProg.FileFd, CheckProg.Buff, BUFFER_SIZE);
        if (bytes_read < 0)
        {
            perror("read");
            conn->state = Connection::COMPLETE;
            ResponseStat = SENDING_COMPLETE;
            close(CheckProg.FileFd);
            conn->response->SetMethod(Error);
            conn->response->SetStatusCode(500);
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
    
    if (bytes_sent == 0)
        return;
    if (bytes_sent < 0)
    {
            perror("send");
            conn->state = Connection::COMPLETE;
            close(CheckProg.FileFd);
            return;
    }
    CheckProg.BuffOffs += bytes_sent;
}


bool    CheckFileRD(Connection *conn)
{
    if (conn->UseCgi)
    {    
        conn->request->SetUri(conn->CgiObj->OutFile);
    }
    struct stat FileState;
    if (stat(conn->request->GetUri().c_str(), &FileState) == -1)
    {
        conn->response->SetStatusCode(404);
        conn->response->SetMethod(Error);
        return false;
    }
    if (!S_ISREG(FileState.st_mode))
    {
        conn->response->SetStatusCode(409);
        conn->response->SetMethod(Error);
        return false;
    }
    int fd = open(conn->request->GetUri().c_str(), O_RDONLY);
    if (fd == -1 && conn->response->GetMethod() != Error)
    {
        conn->response->SetStatusCode(403);
        conn->response->SetMethod(Error);
        return false;
    }
    close(fd);
    return true;
}

void    excuteGetMethod(Connection *conn)
{
    if (conn->UseCgi && conn->response->GetMethod() != Error)
    {
        conn->CgiObj->ExecuteCgi(conn);
        if (conn->CgiObj->IsCgiComplet(conn) == true)
            conn->CgiObj->Pid = -42;
        else
            return ;
    }
    if (conn->response->GetMethod() == GET)
    {
        if (CheckFileRD(conn))
        {
            ExecuteGET(conn);
        }
    }
    if (conn->response->GetMethod() == POST)
    {
        PostResponse(conn);
    }
    if (conn->response->GetMethod() == DELETE)
    {
        ExecuteDelete(conn);
    }
    if (conn->response->GetMethod() == Error)
    {
        ExecuteError(conn);
    }
    // if (conn->UseCgi && conn->state == Connection::COMPLETE)
    // {
    //     unlink(conn->CgiObj->OutFile.c_str());
    // }
}


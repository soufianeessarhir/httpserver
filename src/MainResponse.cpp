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
#include <ctime>
#include <dirent.h>
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
    autoindex = false;
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
        if (conn->request->headers["connection"] != "Keep-alive")
            Headers["Connection"] = "close\r\n";     
        else
            Headers["Connection"] = "keep-alive\r\n";
    }
    else
        Headers["Connection"] = "close\r\n";
    std::ostringstream oss;
    oss << ContentLength;
    if (!conn->CgiObj)
    {
        Headers["Content-Type"] = ContentType + "\r\n";
    }
    // else
    //     Headers["Content-Type"] = "text/html\r\n";
    Headers["Content-Length"] = oss.str() + "\r\n";
}

MainResponse::~MainResponse()
{
    
}

void MainResponse::SendStatusLine(Connection *conn)
{
    if (conn->location && conn->location->has_redirect)
    {
        std::stringstream redirect_response;
        int status_code = conn->location->redirect.first;
        std::string location = conn->location->redirect.second;
        
        // Build proper redirect response
        redirect_response << "HTTP/1.1 " << status_code;
        std::map<int, std::string>::const_iterator it = ErrorPhrase.find(status_code);
        if (it != ErrorPhrase.end())
            redirect_response << " " << it->second << "\r\n";
        else
            redirect_response << " Found\r\n";
            
        redirect_response << "Location: " << location << "\r\n";
        redirect_response << "Content-Length: 0\r\n";
        redirect_response << "Connection: close\r\n";
        redirect_response << "\r\n";
        
        StatusLine = redirect_response.str();
        std::cout << "Redirect Response:\n" << StatusLine << std::endl;
    }
    if (conn->CgiObj)
    {
        std::string location = conn->CgiObj->CgiHeaders["Location"];
        if (!location.empty())
            StatusLine = "HTTP/1.1 302 Found\r\nLocation: " + location + "\r\nConnection:close\r\n\r\n";
        // std::cout << location << "'---------'" << std::endl;
    }
    ssize_t BytesWriten = 0;
    size_t TotalSent = 0;
    
    while (TotalSent < StatusLine.size())
    {
        BytesWriten = send(conn->fd, StatusLine.c_str() + TotalSent, 
                          StatusLine.size() - TotalSent, MSG_NOSIGNAL);
        if (BytesWriten == 0)
        {
            conn->state = Connection::SENDING_RESPONSE;
            return;
        }
        if (BytesWriten < 0)
        {
            // perror("send status line");
            conn->state = Connection::COMPLETE;
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
    if (conn->CgiObj && (conn->request->GetMethod() == "POST" || conn->request->GetMethod() == "GET") 
        && !conn->CgiObj->CgiHeaders.empty())
    {
       std::map<std::string, std::string>::const_iterator itc(conn->CgiObj->CgiHeaders.begin());
       for(;itc != conn->CgiObj->CgiHeaders.end(); ++itc)
            HeadersStr += itc->first + ": " + itc->second + "\r\n";
    }
    HeadersStr += "\r\n";
    // std::cout << HeadersStr << std::endl;
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
            // perror("send headers");
            conn->state = Connection::COMPLETE;
            return;
        }
        TotalSent += static_cast<size_t>(BytesWriten);
    }
}

bool    MainResponse::CheckForSending(Connection *conn)
{
    struct stat FileState;
    if (stat(conn->request->GetUri().c_str(), &FileState) == -1)
    {
        if (conn->response->GetMethod() == Error)
            conn->request->SetUri("Indexes/NotFound.html");
        else
            conn->response->SetMethod(Error);
        conn->response->SetStatusCode(404);
        return false;
    }
    
    if (S_ISDIR(FileState.st_mode) && conn->response->GET->ResponseStat == SENDING_STATUSLINE)
    {
        if (conn->location && !conn->location->index.empty())
            conn->request->SetUri(conn->location->index);
        else if (conn->location && conn->location->autoindex)
        {
            std::string autoindexHTML = conn->response->GET->GenerateAutoIndex(conn);
            if (autoindexHTML.empty())
                return false;
            std::stringstream filefd;
            filefd << conn->fd;
            std::string tempFile = "/tmp/autoindex_" + filefd.str() + ".html";
            std::ofstream file(tempFile.c_str());
            if (file.is_open())
            {
                file << autoindexHTML;
                file.close();
                conn->request->SetUri(tempFile);
                conn->response->GET->autoindex = true;
            }
            else
            {
                conn->response->SetStatusCode(500);
                conn->response->SetMethod(Error);
                return false;
            }
        }
        else
        {
            conn->response->SetStatusCode(403);
            conn->response->SetMethod(Error);
            return false;
        }
    }
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
            conn->state = Connection::COMPLETE;
            close(CheckProg.FileFd);
            return;
    }
    CheckProg.BuffOffs += bytes_sent;
}

std::string MainResponse::GenerateAutoIndex(Connection *conn)
{
    std::ostringstream html;
    std::string uri = conn->request->GetUri();
    std::string path = conn->request->GetUri();

    if (!uri.empty() && uri[uri.length() - 1] != '/')
    {
        int pos = uri.find_last_of('/');
        std::string dir = uri.substr(pos);
        std::ostringstream response;
        response << "HTTP/1.1 301 Moved Permanently\r\n";
        response << "Location: " << dir << "/\r\n";
        response << "Content-Length: 0\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        std::string responseStr = response.str();
        ssize_t bytes_sent = send(conn->fd, responseStr.c_str(), responseStr.length(), MSG_NOSIGNAL);
        if (bytes_sent < 0) {
            return "";
        }
        
        return "";
    }

    std::cout << uri << std::endl;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<title>Index of " << uri << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { color: #333; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "a { color: #0066cc; text-decoration: none; }\n";
    html << "a:hover { text-decoration: underline; }\n";
    html << "</style>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << uri << "</h1>\n";
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";
    if (uri != "/") {
        html << "<tr><td><a href=\"../\">../</a></td><td>-</td><td>-</td></tr>\n";
    }
    
    DIR *dir = opendir(path.c_str());
    if (dir != NULL) 
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            std::string fullPath = path + entry->d_name;
            struct stat fileStat;
            if (stat(fullPath.c_str(), &fileStat) == 0)
            {
                std::string name = entry->d_name;
                if (S_ISDIR(fileStat.st_mode))
                    name += "/";
                std::string size;
                if (S_ISDIR(fileStat.st_mode))
                    size = "-";
                else
                {
                    std::ostringstream sizeStr;
                    sizeStr << fileStat.st_size;
                    size = sizeStr.str();
                }
                char timeStr[100];
                struct tm *timeinfo = localtime(&fileStat.st_mtime);
                strftime(timeStr, sizeof(timeStr), "%d-%b-%Y %H:%M", timeinfo);
                
                html << "<tr><td><a href=\"" << entry->d_name;
                if (S_ISDIR(fileStat.st_mode)) html << "/";
                html << "\">" << name << "</a></td>";
                html << "<td>" << size << "</td>";
                html << "<td>" << timeStr << "</td></tr>\n";
            }
        }
        closedir(dir);
    }
    
    html << "</table>\n</body>\n</html>\n";
    return html.str();
}


bool    CheckFileRD(Connection *conn)
{
    if (conn->location && conn->location->has_redirect)
        return true;
    if (conn->UseCgi)
    {    
        conn->request->SetUri(conn->CgiObj->OutFile);
        // delete conn->CgiObj;
        // conn->CgiObj = NULL;
        // conn->UseCgi = false;
    }
    struct stat FileState;
    if (stat(conn->request->GetUri().c_str(), &FileState) == -1)
    {
        conn->response->SetStatusCode(404);
        conn->response->SetMethod(Error);
        return false;
    }
    if (S_ISDIR(FileState.st_mode))
        return true;
    int fd = open(conn->request->GetUri().c_str(), O_RDONLY);
    if (fd == -1 && conn->response->GetMethod() != Error)
    {
        if (conn->CgiObj)
        {
            conn->response->SetStatusCode(500);
            conn->response->SetMethod(Error);
        }
        else
        {
            conn->response->SetStatusCode(403);
            conn->response->SetMethod(Error);
        }
        return false;
    }
    close(fd);
    return true;
}
void    excuteGetMethod(Connection *conn)
{
    if (conn->CgiObj && conn->response->GetMethod() != Error)
    {
        
        if (conn->CgiObj->ExecuteCgi(conn) == false)
        {
            if (conn->CgiObj)
            {
                delete conn->CgiObj;
                conn->CgiObj = NULL;
                conn->UseCgi = false;
            }
            return;
        }
        if (conn->CgiObj->IsCgiComplet(conn) == false)
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
    if (conn->UseCgi && conn->state == Connection::COMPLETE)
    {
        removeFile(conn->CgiObj->OutFile.c_str());
    }
}


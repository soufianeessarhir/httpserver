/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MainResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:23:23 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/29 11:00:16 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MAINRESPONSE_HPP
#define MAINRESPONSE_HPP

#ifdef __linux__
    #include <sys/socket.h>
#elif defined(__APPLE__)
    #ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
    #endif
    #include <sys/socket.h>
#endif

// #include "Response.hpp"
#include <fstream>

#include "Connection.hpp"
#include <cstddef>

#define BUFFER_SIZE 5000
#define NOTFOUND "Indexes/NotFound.html"
#define FORBIDDEN "Indexes/Forbidden.html"
#define BADREQUEST "Indexes/BadRequest.html"
#define UNAUTHORIZED "Indexes/Unauthorized.html"
#define URITooLong   "Indexes/URITooLong.html"

enum    Prog
{
    READING_FILE,
    SENDING_FILE,
    COMPLETE
};

struct  SendFile
{
    int FileFd;
    off_t FileOffset;
    size_t  FileSize;
    size_t  BuffSize;
    size_t  BuffOffs;
    Prog    Flag;
    char    Buff[BUFFER_SIZE];
};



enum    State
{
    SENDING_STATUSLINE,
    SENDING_BODY,
    SENDING_COMPLETE
};

class Connection;
class MainResponse
{
    private:
        int                                                 StatusCode;
        ssize_t                                             ContentLength;
        // ssize_t                                          BytesSent;
        std::string                                         FilePath;
        bool                                                IsBinaryFile;
        std::string                                         ContentType;
        std::string                                         StatusLine;
        std::string                                         Body;
        std::map<std::string, std::string>                  Headers;
        static const std::map<int, std::string>             ErrorPhrase;
        static const    std::map<std::string, std::string>  MimeTypes;
    public:
        MainResponse(int statusCode, std::string filePath);
        ~MainResponse();
        
        void    SetHeaders(bool CloseConn, Request *req);
        void    SetContentType();
        void    SetStatusLine();
        void    SetPath(std::string NewPath);
        
        const   std::string& GetBody() const;
        const   std::string& GetContentType() const;
        int     GetStatusCode() const;
        
        void    SendStatusLine(Connection *Conn);
        void    SendHeaders(Connection *Conn);
        
        SendFile    CheckProg;
        State       ResponseStat;
        struct stat FileState;
        
        void    SetAndSendBody(Connection *conn);
        bool    CheckForSending(Connection *conn);
        static const    std::map<int, std::string>          ErrorHtmlPath;

};
    
    void    ExecuteError(Connection *conn);
    void    ExecuteGET(Connection *conn);
    void    ExecuteDelete(Connection *conn);

std::map<std::string, std::string> CreateMimeTypes();
void    excuteGetMethod(Connection *conn);
void    SetIndexCaseError(Connection *conn);
void    PostResponse(Connection *conn);


#endif
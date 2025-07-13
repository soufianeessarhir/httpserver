/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:23:23 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/13 17:36:34 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETMETHODRESPONSE_HPP
#define GETMETHODRESPONSE_HPP


// #include "Response.hpp"
#include <fstream>

#include "Connection.hpp"

#define BUFFER_SIZE 5000

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
    SENDING_HEADERS,
    SENDING_BODY,
    SENDING_COMPLETE
};

class Connection;
class GetMethodResponse
{
    private:
        int                                                 StatusCode;
        ssize_t                                             ContentLength;
        ssize_t                                             BytesSent;
        std::string                                         FilePath;
        bool                                                IsBinaryFile;
        std::string                                         ContentType;
        std::string                                         StatusLine;
        std::string                                         Body;
        std::map<std::string, std::string>                  Headers;
        static const std::map<int, std::string>             ErrorPhrase;
        static const    std::map<std::string, std::string>  MimeTypes;
    public:
        GetMethodResponse(int statusCode, std::string filePath);
        ~GetMethodResponse();
    
        void SetHeaders(bool CloseConn);
        void SetContentType();
        void SetStatusLine();

        const std::string& GetBody() const;
        const std::string& GetContentType() const;
        int GetStatusCode() const;

        void    SendStatusLine(Connection *Conn);
        void    SendHeaders(Connection *Conn);

        SendFile    CheckProg;
        State       ResponseStat;

        void    SetAndSendBody(Connection *conn);
        bool    CheckForSending();
};


std::map<std::string, std::string> CreateMimeTypes();
void    excuteGetMethod(Connection *conn);

#endif
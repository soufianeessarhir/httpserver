/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:49 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/31 11:32:47 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection(int fd): UseCgi(false),state(READING_REQUEST_LINE),fd(fd)
            ,request(new Request()),response(NULL),server(NULL),location(NULL),post(NULL)
{
    timeouts.read_fails = false;
    timeouts.idle_timeout = time(NULL);     
    timeouts.last_activity = time(NULL);     
    timeouts.read_timeout = time(NULL);     
}


Connection::~Connection()
{
    UseCgi = false;
    state = READING_REQUEST_LINE;
    if (request)
    {
        delete request;
        request = NULL;
    }
    if (response)
    {
        delete response;
        response = NULL;
    }
    if (server)
    {
        delete server;
        server = NULL;
    }
    if (location)
    {
        delete location;
        location = NULL;
    }
    if (post)
    {
        delete post;
        post = NULL;
    }
    
}
void    CheckCgiExist(Connection *conn) // add by eaboudi
{
    std::string Path = conn->location->root + conn->request->GetUri();
    std::string QueryString;
    std::string CheckDir;
    std::string Ext;
    size_t Pos = Path.find('?');
    if (Pos != std::string::npos)
    {
        QueryString = Path.substr(Pos + 1);
        Path = Path.substr(0, Pos);
    }
    Pos = Path.find('.');
    if (Pos != Path.npos)
    {
        std::string PathInfo;
        if (Path.find('/', Pos) != Path.npos)
        {
            while(Path[Pos] != '/')
            {
                Ext.push_back(Path[Pos]);
                Pos++;
            }
            PathInfo = Path.substr(Pos);
        }
        else
        {
            Ext = Path.substr(Pos);
            Pos = Path.size();
        }
        CheckDir = Path.substr(0, Pos);
        Pos = CheckDir.find_last_of('/');
        std::string ScriptName = CheckDir.substr(Pos + 1);
        CheckDir = CheckDir.substr(0, Pos);
        std::string CgiDir = CGiDir;
        if (CheckDir.compare(0, CgiDir.size(), CgiDir) == 0)
        {
            CheckDir += '/';
            std::string ScriptPath = CheckDir;
            CheckDir += ScriptName;
            struct stat FileState;
            if (stat(CheckDir.c_str(), &FileState) == 0)
            {
                if (S_ISREG(FileState.st_mode))
                {
                    conn->UseCgi = true;
                    conn->CgiObj = new CGI;
                    conn->CgiObj->Ext = Ext;
                    conn->CgiObj->QUERY_STRING = QueryString;
                    conn->CgiObj->REQUEST_METHOD = conn->request->GetMethod();
                    conn->CgiObj->SCRIPT_PATH = ScriptPath;
                    conn->CgiObj->SCRIPT_NAME = ScriptPath + ScriptName;
                    conn->CgiObj->PATH_INFO = PathInfo;
                    conn->CgiObj->REMOTE_ADDR = conn->ip;
                    conn->CgiObj->REMOTE_PORT = conn->port;
                    conn->CgiObj->SERVER_PROTOCOL = conn->request->GetVersion();
                    conn->CgiObj->REMOTE_IDENT = "webserv";
                    // if (conn->CgiObj->REQUEST_METHOD == "POST")
                    // {
                    //     conn->CgiObj->CONTENT_LENGTH  = conn->request->GetContentLenght();
                    //     conn->CgiObj->CONTENT_TYPE = conn->request->GetHeader("content_type");
                    // }
                    // else
                    // {
                        conn->CgiObj->CONTENT_LENGTH = conn->request->GetHeader("content-lenght");
                        conn->CgiObj->CONTENT_TYPE = conn->request->GetHeader("content-lenght");
                    // }
                    conn->request->SetUri(Path);
                    return ;
                }
            }
        }
    }
    conn->request->SetUri(Path);
    conn->UseCgi = false;
}



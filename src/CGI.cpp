/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/08/24 10:25:01 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include <unistd.h>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <signal.h>
#include <errno.h>

void my_usleep(unsigned int usec) {
    struct timeval tv;
    tv.tv_sec  = usec / 1000000;
    tv.tv_usec = usec % 1000000;
    select(0, NULL, NULL, NULL, &tv);
}

CGI::CGI()
{
    OutFile = "/tmp/cgi_out";
    InSize = 0;
    OutputSize = 0;
    SERVER_PORT = 0;
    Pid = -42;
    Env = NULL;
    SCRIPT_PATH.clear();
    SCRIPT_NAME.clear();
    QUERY_STRING.clear();
    PATH_INFO.clear();
    SERVER_PROTOCOL.clear();
    SERVER_NAME.clear();
    REMOTE_ADDR.clear();
    REMOTE_PORT.clear();
    REMOTE_IDENT.clear();
    CONTENT_TYPE.clear();
    CONTENT_LENGTH.clear();
    REQUEST_METHOD.clear();
    SERVER_PORT = 0;
    Is_Runing = 42;
    State = Start;
    
}

char **    CGI::BuildEnv(Connection *conn)
{
    // std::cerr << "enter to BuildEnvv" << std::endl;
    std::stringstream ss;
    ss << conn->fd;
    if (!PATH_INFO.empty())
        EnvString.push_back("PATH_INFO=" + PATH_INFO);
    if (conn->request->GetMethod() == "POST")
    {
        EnvString.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
        EnvString.push_back("CONTENT_TYPE=" + CONTENT_TYPE);
    }
    // EnvString.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
    EnvString.push_back("PATH_TRANSLATED=" + conn->location->root + conn->CgiObj->PATH_INFO);
    EnvString.push_back("HTTP_USER_AGENT=Client ID:" + ss.str());
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    EnvString.push_back("REQUEST_METHOD=" + conn->request->GetMethod());
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("SERVER_SOFTWARE=Webserv/1.0");
    EnvString.push_back("SCRIPT_PATH=" + SCRIPT_PATH);
    EnvString.push_back("SCRIPT_NAME=CGI_SCRIPTS/Login.php");
    ss.clear();
    ss << SERVER_PORT;
    EnvString.push_back("SERVER_PORT=" + ss.str());
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    ss.clear();
    ss << conn->port;
    EnvString.push_back("REMOTE_PORT=" + ss.str());
    EnvString.push_back("GATEWAY_INTERFACE=CGI/1.1");
    EnvString.push_back("REMOTE_IDENT=Webserv");
    EnvString.push_back("REDIRECT_STATUS=200");
    EnvString.push_back("REMOTE_USER=Webserv");
    EnvString.push_back("SCRIPT_FILENAME=" + SCRIPT_PATH + SCRIPT_NAME);
    std::map<std::string, std::string>::iterator it = conn->request->headers.begin();
    for (; it != conn->request->headers.end(); ++it)
    {
        std::string headerName = it->first;
        std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);
        EnvString.push_back("HTTP_" + headerName + "=" + it->second);
    }
    
    Env = new char*[EnvString.size() + 1];
    for (size_t i(0); i < EnvString.size(); i++)
    {
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    }
    Env[EnvString.size()] = NULL;
    return Env;
}

bool CGI::ExecuteCgi(Connection *conn)
{
    if (State == Runing)
    {
        time_t current(std::time(NULL));
        if (current - start > 10)
        {
            conn->response->SetMethod(Error);
            conn->response->SetStatusCode(504);
            kill(Pid, SIGTERM);
            my_usleep(100000);
            kill(Pid, SIGKILL);
            waitpid(Pid, NULL, WNOHANG);
            State = Finished;
            return false;
        }
        return true;
    }
    if (State == Finished)
        return true;
    std::stringstream id;
    id << conn->fd;
    OutFile += id.str();
    this->Pid = fork();
    if (Pid < 0)
    {
        perror("Fork: ");
        conn->response->SetMethod(Error);
        conn->response->SetStatusCode(500);
        return false;
    }
    this->start = std::time(NULL);
    if (this->Pid == 0)
    {
        if (conn->request->GetMethod() == "POST")
        {
            struct stat FileIn;
            if (stat(InFile.c_str(), &FileIn) == 0)
                InSize = FileIn.st_size;
            else
                exit(EXIT_FAILURE);
            if (conn->location->max_body_size && InSize > conn->location->max_body_size)
                exit(CONTENT_TOO_LARGE);
        }
        char** env = BuildEnv(conn);
        int FdOut = open(OutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (FdOut < 0)
        {
            delete [] Env;
            exit(EXIT_FAILURE);
        }
        if (dup2(FdOut, STDOUT_FILENO) < 0)
        {
            delete [] Env;
            close(FdOut);
            exit(EXIT_FAILURE);
        }
        close(FdOut);
        if (conn->response->GetMethod() == POST)
        {
            int FdIn = open(InFile.c_str(), O_RDONLY);
            if (FdIn < 0)
            {
                delete [] Env;
                exit(EXIT_FAILURE);
            }
            if (dup2(FdIn, STDIN_FILENO) < 0)
            {
                close(FdIn);
                delete [] Env;
                exit(EXIT_FAILURE);
            }
            close(FdIn);
        }
        std::string Script = SCRIPT_PATH + SCRIPT_NAME;
        char *argv[3] = {const_cast<char*>(conn->location->cgi[Ext].c_str()), const_cast<char*>(Script.c_str()), NULL};
        
        if (execve(argv[0],(&argv[0]), env) == -1)
        {
            perror("execeve: ");
            delete [] Env;
            exit(EXIT_FAILURE);
        }
    }
    State = Runing;
    return true;
}

bool    CGI::IsCgiComplet(Connection *conn)
{ 
    if (State == Finished)
        return true;
    
    int Status;
    pid_t   Res = waitpid(Pid, &Status, WNOHANG);
    if (Res == 0)
        return false;
    if (Res == -1)
    {
        conn->response->SetMethod(Error);
        conn->response->SetStatusCode(500);
        State = Finished;
        return true;
    }
    State = Finished;
    if (Res == Pid)
    {
        std::fstream    OFile(OutFile.c_str(), std::ios::in);
        if (OFile)
        {
            std::stringstream buff;
            std::string line;
            buff << OFile.rdbuf();
            while (std::getline(buff, line))
            {
                if (line.empty() || line.find(':') == line.npos)
                    break;
                size_t  Pos(line.find('\r'));
                if (Pos != line.npos)
                    line = line.substr(0, Pos);
                Pos = line.find(':');
                if (Pos == line.npos)
                    break;
                if (Pos + 1 >= line.size())
                    continue;
                std::string Key(line.substr(0, Pos));
                std::string Value(line.substr(Pos + 1));
                CgiHeaders[Key] = Value;
            }
            size_t DoubleCrCf(buff.str().find("\r\n\r\n"));
            std::string content;
            if (DoubleCrCf != buff.str().npos)
                content = buff.str().substr(DoubleCrCf + 4);
            else
                content = buff.str();
            OFile.close();
            OutputSize = content.size();
            OFile.open(OutFile.c_str(), std::ios::trunc | std::ios::out);
            OFile << content;
            OFile.close();
            conn->response->SetStatusCode(200);
            kill(Pid, SIGKILL);
            while (waitpid(Pid, &Status, WNOHANG) >= 0)
                ;
        }
        else
        {
            conn->response->SetMethod(Error);
            conn->response->SetStatusCode(500);
        }
    }
    if (conn->response->GetMethod() == POST)
        removeFile(InFile.c_str());
    return true;
}

CGI::~CGI()
{
    if (Pid > 0 && State != Finished)
    {
        kill(Pid, SIGTERM);
        my_usleep(100000);
        if (waitpid(Pid, NULL, WNOHANG) == 0)
        {
            kill(Pid, SIGKILL);
            waitpid(Pid, NULL, 0);
        }
    }
    if (!OutFile.empty())
        removeFile(OutFile.c_str());
    if (!InFile.empty())
        removeFile(InFile.c_str());
}
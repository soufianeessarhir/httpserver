/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/08/18 18:21:48 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include <unistd.h>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <signal.h>

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
    
}

char **    CGI::BuildEnv(Connection *conn)
{
    std::cerr << "enter to BuildEnvv" << std::endl;
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
        std::cout << Env[i] << std::endl;
    }
    Env[EnvString.size()] = NULL;
    return Env;
}

bool CGI::ExecuteCgi(Connection *conn)
{
    if (Is_Runing == 1)
    {
        // std::cout << "is runing---->" << Is_Runing << std::endl;
        time_t current(std::time(NULL));
        // std::cout << "start time----> " << start << std::endl;
        // std::cout << "current time----> " << current << std::endl;
        if (current - start > 10)
        {
            conn->response->SetMethod(Error);
            conn->response->SetStatusCode(504);
            kill(Pid, SIGTERM);
            return false;
        }
        return true;
    }
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
            std::cout << "dkhol ydupi input" << std::endl;
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
            std::cerr << "fdout" << std::endl;
            delete [] Env;
            exit(EXIT_FAILURE);
        }
        if (dup2(FdOut, STDOUT_FILENO) < 0)
        {
            std::cerr << "fdout" << std::endl;
            delete [] Env;
            close(FdOut);
            exit(EXIT_FAILURE);
        }
        close(FdOut);
        if (conn->response->GetMethod() == POST)
        {
            std::cerr << "dkhol l dup infile\n";
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
        // std::cerr << "fdout" << std::endl;
        std::string Script = SCRIPT_PATH + SCRIPT_NAME;
        char *argv[3] = {const_cast<char*>(conn->location->cgi[Ext].c_str()), const_cast<char*>(Script.c_str()), NULL};
        if (execve(argv[0],(&argv[1]), env) == -1)
        {
            perror("execeve: ");
            delete [] Env;
            exit(EXIT_FAILURE);
        }
    }
    Is_Runing = 1;
    return true;
}

bool    CGI::IsCgiComplet(Connection *conn)
{ 
    // sleep(1);
    int Status;
    pid_t   Res = waitpid(Pid, &Status, WNOHANG);
    // std::cout << "Res :" << Res << std::endl;
    if (Res == 0)
        return false;
    Is_Runing = 0;
    if (Res == Pid)
    {
         std::cout << "here" << std::endl;
        std::fstream    OFile(OutFile.c_str(), std::ios::in);
        // kill(Pid, SIGKILL);
        if (OFile)
        {
            std::cout << "entered to Ofile Condition fd =" << conn->fd << std::endl;
            std::stringstream buff;
            std::string line;
            buff << OFile.rdbuf();
            // std::cout << "-------\n" << buff.str() << "\n---------" << std::endl;
            while (std::getline(buff, line))
            {
                std::cout << "enter to the loop" << std::endl;
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
        }
    }
    else if (Res == -1)
    {
        conn->response->SetMethod(Error);
        conn->response->SetStatusCode(500);
        kill(conn->CgiObj->Pid, SIGTERM);
    }
    if (conn->response->GetMethod() == POST)
        unlink(InFile.c_str());
    return true;
}

CGI::~CGI()
{

}
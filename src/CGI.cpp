/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/31 06:36:29 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{
    
}

void    CGI::BuildEnv(Connection *conn)
{
    std::stringstream ss;
    ss << conn->fd;
    EnvString.push_back("PATH_TRANSLATED=" + conn->location->root + conn->CgiObj->PATH_INFO);
    EnvString.push_back("HTTP_USER_AGENT=Client ID:" + ss.str());
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    EnvString.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
    EnvString.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
    EnvString.push_back("CONTENT_TYPE=" + CONTENT_TYPE);
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("SERVER_SOFTWARE=Webserv/1.0");
    EnvString.push_back("SCRIPT_PATH=" + SCRIPT_PATH);
    EnvString.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
    EnvString.push_back("SERVER_NAME=" + SERVER_NAME);
    EnvString.push_back("SERVER_PORT=" + SERVER_PORT);
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    EnvString.push_back("REMOTE_PORT=" + REMOTE_PORT);
    EnvString.push_back("GATEWAY_INTERFACE=CGI/1.1");
    EnvString.push_back("PATH_INFO=" + PATH_INFO);
    EnvString.push_back("REMOTE_IDENT=Webserv");
    EnvString.push_back("REDIRECT_STATUS=200");
    EnvString.push_back("REMOTE_USER=Webserv");

    Env = new char*[EnvString.size() + 1];

    for (size_t i(0); i < EnvString.size(); i++)
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    Env[EnvString.size()] = NULL;
}

void CGI::ExecuteCgi(Connection *conn)
{
    std::stringstream id;
    id << conn->fd;
    OutFile = "/tmp/CgiOutFile" +  id.str();
    Pid = fork();
    if (Pid == 0)
    {
        if (conn->request->GetMethod() == "POST")
        {
            struct stat FileIn;
            if (stat(InFile.c_str(), &FileIn) == 0)
                InSize = FileIn.st_size;
            else
                exit(EXIT_FAILURE);
        }
        //check if the content too large
        BuildEnv(conn);
        int FdOut = open(InFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
        if (conn->request->GetMethod() == "POST")
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
    }
    Is_Runing = true;
}



CGI::~CGI()
{
    
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/08/05 21:24:16 by eaboudi          ###   ########.fr       */
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
    ss.clear();
    ss << SERVER_PORT;
    EnvString.push_back("SERVER_PORT=" + ss.str());
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
    std::cout << "execute success" << std::endl;
    
    // std::stringstream id;
    // id << conn->fd;
    // OutFile = "/tmp/CgiOutFile" + id.str();
    // Pid = fork();
    // if (Pid == 0)
    // {
    //     if (conn->request->GetMethod() == "POST")
    //     {
    //         struct stat FileIn;
    //         if (stat(InFile.c_str(), &FileIn) == 0)
    //             InSize = FileIn.st_size;
    //         else
    //             exit(EXIT_FAILURE);
    //     }
    //     //check if the content too large
    //     BuildEnv(conn);
    //     int FdOut = open(OutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //     if (FdOut < 0)
    //     {
    //         delete [] Env;
    //         exit(EXIT_FAILURE);
    //     }
    //     if (dup2(FdOut, STDOUT_FILENO) < 0)
    //     {
    //         delete [] Env;
    //         close(FdOut);
    //         exit(EXIT_FAILURE);
    //     }
    //     close(FdOut);
    //     if (conn->response->GetMethod() == POST)
    //     {
    //         int FdIn = open(InFile.c_str(), O_RDONLY);
    //         if (FdIn < 0)
    //         {
    //              delete [] Env;
    //             exit(EXIT_FAILURE);
    //         }
    //         if (dup2(FdIn, STDIN_FILENO) < 0)
    //         {
    //             close(FdIn);
    //             delete [] Env;
    //             exit(EXIT_FAILURE);
    //         }
    //         close(FdIn);
    //     }
    //     const char * argv[] = {"php", SCRIPT_NAME.c_str(), NULL};
    //     if (execve("php", const_cast<char **>(argv), Env) == -1)
    //     {
    //         delete [] Env;
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // Is_Runing = true;
}

bool    CGI::IsCgiComplet(Connection *conn)
{
    (void)conn;
    if (!Is_Runing)
        return true;
    int Status;
    pid_t   Res = waitpid(Pid, &Status, WNOHANG);
    if (!Res)
        return false;
    Is_Runing = false;
    if (Res == -1)
    {
        conn->response->SetStatusCode(500);
        conn->response->SetMethod(Error);
        return true;
    }
    if (WIFSIGNALED(Status) && WTERMSIG(Status) == SIGALRM)
    {
        conn->response->SetStatusCode(504);
        conn->response->SetMethod(Error);
    }
    else if (WIFEXITED(Status) && WEXITSTATUS(Status) == 0)
	{
        std::fstream    OFile(OutFile.c_str());
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
            OutputSize = content.size();
            conn->response->SetStatusCode(200);
            OFile.close();
        }
        else
        {
            conn->response->SetStatusCode(500);
            conn->response->SetMethod(Error);
        }
    }
    else if (WIFEXITED(Status) && WEXITSTATUS(Status) == CONTENT_TOO_LARGE)
    {
        conn->response->SetStatusCode(413);
        conn->response->SetMethod(Error);
    }
    else
    {
        conn->response->SetStatusCode(500);
        conn->response->SetMethod(Error);
    }
    if (conn->response->GetMethod() == POST)
        unlink(InFile.c_str());
    return true;
}

CGI::~CGI()
{
    
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/08/16 19:42:14 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include <unistd.h>

CGI::CGI()
{
    OutFile = "/tmp/cgi_out";
    InSize = 0;
    OutputSize = 0;
    SERVER_PORT = 0;
    Pid = -42;
    Env = NULL;
}

void    CGI::BuildEnv(Connection *conn)
{
    std::stringstream ss;
    ss << conn->fd;
    EnvString.push_back("PATH_TRANSLATED=" + conn->location->root + conn->CgiObj->PATH_INFO);
    EnvString.push_back("HTTP_USER_AGENT=" + conn->request->GetHeader("user-agent"));
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    EnvString.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
    EnvString.push_back("REQUEST_METHOD=" + conn->request->GetMethod());
    EnvString.push_back("CONTENT_TYPE=" + CONTENT_TYPE);
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("SERVER_SOFTWARE=Webserv/1.0");
    EnvString.push_back("SCRIPT_PATH=" + SCRIPT_PATH);
    EnvString.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
    EnvString.push_back("SERVER_NAME=localhost");
    EnvString.push_back("DOCUMENT_ROOT=" + conn->location->root);
    ss.clear();
    ss << conn->port;
    EnvString.push_back("SERVER_PORT=" + ss.str());
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    EnvString.push_back("REMOTE_PORT=" + REMOTE_PORT);
    EnvString.push_back("GATEWAY_INTERFACE=CGI/1.1");
    EnvString.push_back("PATH_INFO=" + PATH_INFO);
    EnvString.push_back("REMOTE_IDENT=Webserv");
    EnvString.push_back("REDIRECT_STATUS=200");
    EnvString.push_back("REMOTE_USER=Webserv");
    EnvString.push_back("SCRIPT_FILENAME=" + SCRIPT_PATH + SCRIPT_NAME);
    for (std::map<std::string,std::string>::iterator it = conn->request->headers.begin(); it != conn->request->headers.end(); ++it)
    {
        std::string headerName = it->first;
        std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);
        std::replace(headerName.begin(), headerName.end(), '-', '_');
        EnvString.push_back("HTTP_" + headerName + "=" + it->second);
    }

    for (size_t i(0); i < EnvString.size(); i++)
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    Env[EnvString.size()] = NULL;
}

void CGI::ExecuteCgi(Connection *conn)
{
    if (Pid != -42)
        return ;
    std::stringstream id;
    id << conn->fd;
    OutFile += id.str();
    this->Pid = fork();
    if (this->Pid == 0)
    {
        // if (conn->request->GetMethod() == "POST")
        // {
        //     struct stat FileIn;
        //     if (stat(InFile.c_str(), &FileIn) == 0)
        //         InSize = FileIn.st_size;
        //     else
        //         exit(EXIT_FAILURE);
        //     if (conn->location->max_body_size && InSize > conn->location->max_body_size)
        //         exit(CONTENT_TOO_LARGE);
        //     std::cerr << "checking infile finished" << std::endl;
        // }
        BuildEnv(conn);
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
        // if (conn->request->GetMethod() == "POST")
        // {
           
        //     int FdIn = open(InFile.c_str(), O_RDONLY);
        //     if (FdIn < 0)
        //     {
        //         delete [] Env;
        //         perror("");
        //         exit(EXIT_FAILURE);
        //     }
        //     if (dup2(FdIn, STDIN_FILENO) < 0)
        //     {
        //         perror("");
        //         close(FdIn);
        //         delete [] Env;
        //         exit(EXIT_FAILURE);
        //     }
        //     close(FdIn);
        //     std::cerr << "duping infile" << std::endl;
        // }
        std::string Script = SCRIPT_PATH + SCRIPT_NAME;
        char *argv[3] = {const_cast<char*>(conn->location->cgi[Ext].c_str()), const_cast<char*>(Script.c_str()), NULL};
        if (execve(argv[0],(&argv[1]), Env) == -1)
        {
            perror("execeve: ");
            delete [] Env;
            exit(EXIT_FAILURE);
        }
    }
    Is_Runing = 1;
}

bool    CGI::IsCgiComplet(Connection *conn)
{    
    if (Is_Runing == 0)
        return true;
    int Status;
    pid_t   Res = waitpid(Pid, &Status, WNOHANG);
    std::cout << "Res :" << Res << std::endl;
    // if (!Res)
    //     return false;
    Is_Runing = 0;
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
   if (WIFEXITED(Status) && WEXITSTATUS(Status) == 0)
	{
        std::cout << "here" << std::endl;
        std::fstream    OFile(OutFile.c_str(), std::ios::in);
        if (OFile)
        {
            kill(Pid, SIGKILL);
            std::stringstream buff;
            std::string line;
            buff << OFile.rdbuf();
            std::cout << "------------------" << buff.str() << "---------------" << std::endl;
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
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/29 12:02:47 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{
    
}

void    CGI::BuildEnv()
{
    EnvString.push_back("SCRIPT_PATH=" + SCRIPT_PATH);
    EnvString.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
    EnvString.push_back("PATH_INFO=" + PATH_INFO );
    EnvString.push_back("CONTENT_TYPE=" + CONTENT_TYPE);
    // EnvString.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    EnvString.push_back("SERVER_NAME=" + SERVER_NAME);
    EnvString.push_back("SERVER_PORT=" + SERVER_PORT);
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    // EnvString.push_back("REMOTE_PORT=" + REMOTE_PORT);
    Env = new char*[EnvString.size() + 1];
    for (size_t i(0); i < EnvString.size(); i++)
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    Env[EnvString.size()] = NULL;
}

void CGI::ExecuteCgi()
{
    Out_File = "CGI-SCRIPTS/test";
    SCRIPT_FDO = open(Out_File.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (SCRIPT_FDO == -1) 
    {
        perror("open");
        return;
    }
    BuildEnv();
    int Child = fork();
    if (Child == -1) 
    {
        perror("fork");
        return;
    }

    // if (Child == 0)
    // {
    //     if (dup2(SCRIPT_FDO, STDOUT_FILENO) == -1)
    //     {
    //         perror("dup2");
    //         exit(EXIT_FAILURE);
    //     }
    //     if (REQUEST_METHOD == "POST")
    //     {
    //             struct stat InFile;
    //             // stat(PostBodyFile.c_str(), &InFile);
    //             stat("CGI-SCRIPTS/hello.txt", &InFile);
    //             CONTENT_LENGTH =  InFile.st_size;
    //             // int FdIn = open(PostBodyFile.c_str(), O_RDONLY);
    //             int FdIn = open("CGI-SCRIPTS/hello.txt", O_RDONLY);
    //             if (dup2(FdIn, STDIN_FILENO) == -1)
    //             {
    //                 perror("dup2");
    //                 exit(EXIT_FAILURE);
    //             }
    //             char buffer[1024];
    //             // ssize_t bytesRead;
    //             read(FdIn, buffer, sizeof(buffer));
    //             std::cout << buffer << std::endl;
    //             close(FdIn);
    //     }
    //     char *argv[] = {(char*)"CGI-SCRIPTS/cgi.sh", NULL};
    //     if (execve("CGI-SCRIPTS/cgi.sh", argv, Env) == -1)
    //     {
    //         perror("execve");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // else
    // {
    //     int status;
    //     waitpid(Child, &status, 0);
    // }
}

CGI::~CGI()
{
    delete[] Env;
    close(SCRIPT_FDO);
}
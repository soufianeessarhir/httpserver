/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/10 11:21:10 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{
    // REQUEST_METHOD = Conn->request->GetMethod();
    // Uri = Conn->request->GetUri();
    // DecodeUri();
    // if (Uri.find('#') != std::string::npos)
    //     Uri = Uri.substr(0, Uri.find('#'));
    // if (Uri.find('?') != std::string::npos)
    // {
    //     SCRIPT_PATH = Uri.substr(0, Uri.find('?'));
    //     QUERY_STRING = Uri.substr(Uri.find('?') + 1);
    // }
    // else
    // {
    //     SCRIPT_PATH = Uri;
    //     QUERY_STRING = "";
    // }
    // struct sockaddr_in ServerAddr;
    // socklen_t AddrLen = sizeof(ServerAddr);
    // std::string ServerIp;
    // if (!getsockname(Conn->fd, (struct sockaddr*)&ServerAddr, &AddrLen))
    // {
    //     ServerIp = inet_ntoa(ServerAddr.sin_addr);

    //     std::ostringstream PortStream;
    //     PortStream << ntohs(ServerAddr.sin_port);
    //     SERVER_PORT = PortStream.str();
    //     SERVER_NAME = Conn->request->GetHeader("host");
    //     if (SERVER_NAME.empty())
    //         SERVER_NAME = ServerIp;
    // }
    // REMOTE_ADDR = Conn->ip;
    // REMOTE_PORT = Conn->port;
    // SERVER_PROTOCOL = Conn->request->GetVersion();
}

void    CGI::BuildEnv()
{
    // std::vector<std::string> EnvString;
    // EnvString.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
    // EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    // EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    // EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    // Env = new char*[EnvString.size() + 1];
    // for (int i(0); i < EnvString.size(); i++)
    //     Env[i] = const_cast<char *>(EnvString[i].c_str());
    // Env[EnvString.size()] = NULL;
}

void    CGI::ExecuteCgi()
{
    // Vars.Child = fork();
    // if (Vars.Child == -1)
    //     return;
    // BuildEnv();
    // if (Vars.Child == 0)
    // {
    //     execve("/home/eaboudi/Desktop/httpserver/bin/cgi.sh", NULL, Env);
    // }
}

CGI::~CGI()
{
    //delete[] Env
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/07 09:56:16 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(Connection *Conn)
{
    Method = Conn->response->GetMethod();
    std::string Uri = Conn->request->GetUri();
    if (Uri.find('?') != std::string::npos)
    {
        SCRIPT_PATH = Uri.substr(0, Uri.find('?'));
        QUERY_STRING = Uri.substr(Uri.find('?') + 1);
    }
    else
    {
        SCRIPT_PATH = Uri;
        QUERY_STRING = "";
    }
    struct sockaddr_in ServerAddr;
    socklen_t AddrLen = sizeof(ServerAddr);
    std::string ServerIp;
    if (!getsockname(Conn->fd, (struct sockaddr*)&ServerAddr, &AddrLen))
    {
        ServerIp = inet_ntoa(ServerAddr.sin_addr);

        std::ostringstream PortStream;
        PortStream << ntohs(ServerAddr.sin_port);
        SERVER_PORT = PortStream.str();
        SERVER_NAME = Conn->request->GetHeader("host");
        if (SERVER_NAME.empty())
            SERVER_NAME = ServerIp;
    }
    REMOTE_ADDR = Conn->ip;
    REMOTE_PORT = Conn->port;
    SERVER_PROTOCOL = Conn->request->GetVersion();
}


int    Hextoint(int c)
{
    if (c <= '0' && c <= '9')
        return c - '0';
    if (c <= 'a' && c <= 'y')
        return c - 'a' + 10;
    if (c <= 'A' && c <= 'Y')
        return c - 'A' + 10;
    return -1;
}

char DecodeIntToChar(char A, char B)
{
    int H(Hextoint(A));
    int L(Hextoint(B));
    
    if (H < 0 || L < 0)
        return '[';
    int Result(H << 4) | L;
    return (static_cast<char>(Result));
}

void    CGI::ParseQueryString()
{
    int Pos;
    for(int i(0); i < QUERY_STRING.size(); i++)
    {
        Pos = QUERY_STRING.find(i, '%');
        if (Pos == QUERY_STRING.npos)
            break;
    }
}
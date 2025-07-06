/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/06 18:30:49 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"




CGI::CGI(Connection *Conn)
{
    Method = Conn->response->GetMethod();
    std::string Uri = Conn->request->GetUri();
    if (Uri.find('?') != std::string::npos)
    {
        ScritpPath = Uri.substr(0, Uri.find('?'));
        QueryString = Uri.substr(Uri.find('?') + 1);
    }
    else
    {
        ScritpPath = Uri;
        QueryString = "";
    }
    struct sockaddr_in ServerAddr;
    socklen_t AddrLen = sizeof(ServerAddr);
    std::string ServerIp;
    if (!getsockname(Conn->fd, (struct sockaddr*)&ServerAddr, &AddrLen))
    {
        ServerIp = inet_ntoa(ServerAddr.sin_addr);

        std::ostringstream PortStream;
        PortStream << ntohs(ServerAddr.sin_port);
        ServerPort = PortStream.str();
        ServerName = Conn->request->GetHeader("host");
        if (ServerName.empty())
            ServerName = ServerIp;
    }
    
}
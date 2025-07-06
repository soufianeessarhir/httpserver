/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:07:58 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/06 18:19:30 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             CGI_HPP
#define             CGI_HPP

#include "Connection.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>

class CGI
{
    // private:
    public:
        std::string     ScritpPath;
        std::string     QueryString;
        Methods         Method;
        std::string     ContentType;
        std::string     Body;
        ssize_t         ContentLength;
        std::string     ServerProtocol;
        std::string     ServerName;
        std::string     ServerPort;
        std::string     RemoteAddr;
        std::string     RemotePort;
        CGI(Connection *Conn);
        ~CGI();
        
    
};

#endif
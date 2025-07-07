/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:07:58 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/07 09:57:12 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             CGI_HPP
#define             CGI_HPP

#include <netinet/in.h>
#include "Connection.hpp"
#include <arpa/inet.h>

class Connection;
class CGI
{
    // private:
    public:
        std::string     SCRIPT_PATH;
        std::string     QUERY_STRING;
        Methods         METHOD;
        std::string     CONTENT_TYPE;
        ssize_t         CONTENT_LENGTH;
        std::string     BODY;
        std::string     SERVER_PROTOCOL;
        std::string     SERVER_NAME;
        std::string     SERVER_PORT;
        std::string     REMOTE_ADDR;
        int             REMOTE_PORT;
        std::map<std::string, std::string> QUERY_PARAMS;
        
        
        CGI(Connection *Conn);
        ~CGI();
        void            ParseQueryString();
};

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/05 12:28:12 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include "ConfigData.hpp"
#include "Request.hpp"
#include "GetMethodResponse.hpp"
#include "Connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
class Connection;
#define HttpVersion "HTTP/1.1 "
std::map<int, std::string> createErrorPhrase(void);
enum Methods
{
    GET,
    POST,
    DELETE,
    Error,
};
class GetMethodResponse;

class Response
{
    public:
        Response();
        Response(int StatusCode, Methods _Method);
        Response(Request *, Server *);
        ~Response();
        GetMethodResponse *GET;
        int GetStatusCode() const;
        int GetMethod() const;
        void    SetMethod(Methods method);
        void    ErrorResponse(Connection *Conn);
        static const std::map<int, std::string> ErrorPhrase;
    private:
        Methods Method;
        int StatusCode;
        std::string StatusLineError;
    
};

std::map<int, std::string> createErrorPhrase();

#endif
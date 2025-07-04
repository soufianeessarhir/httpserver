/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/18 13:11:04 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sys/socket.h>
#include <map>
#include <sstream>
#include "ConfigData.hpp"
#include "Request.hpp"
#include "GetMethodResponse.hpp"
#include "Connection.hpp"
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
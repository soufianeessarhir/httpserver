/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/19 08:54:09 by eaboudi          ###   ########.fr       */
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
#include "MainResponse.hpp"
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
class MainResponse;

class Response
{
    public:
        Response();
        Response(int StatusCode, Methods _Method);
        // Response(Request *, Server *);
        ~Response();
        MainResponse *GET;
        int GetStatusCode() const;
        Methods GetMethod() const;
        void    SetMethod(Methods method);
        static const std::map<int, std::string> ErrorPhrase;
        void    CheckCgiExist();
    private:
        Methods Method;
        int StatusCode;
        std::string StatusLineError;
        
    
};

std::map<int, std::string> createErrorPhrase();

#endif
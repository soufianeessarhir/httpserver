/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/11 23:11:45 by eaboudi          ###   ########.fr       */
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

#define HttpVersion "HTTP/1.1 "
std::map<int, std::string> createErrorPhrase(void);

class GetMethodResponse;

class Response
{
    public:
        Response(int errorCode) : ErrorCode(errorCode) {};
        Response(Request &, Server &);
        ~Response();
        GetMethodResponse *GET;
        int ErrorCode;
    private:
       
};


#endif
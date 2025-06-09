/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/09 16:52:46 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include "ConfigData.hpp"
#include "Request.hpp"

#define HttpVersion "HTTP/1.1 "
std::map<int, std::string> createErrorPhrase(void);
class Response
{
    public:
        Response(int status);
        Response(Request &, Server &);
        ~Response();

        int GetStatus()const;
        std::string& GetData();
        void SetData(const std::string &data);
        std::string BuildResponse();
    private:
        int StatusCode;
        std::map<std::string, std::string> headers;
        static const std::map<int, std::string> ErrorPhrase;
        std::string data;
        std::string body;
};


#endif
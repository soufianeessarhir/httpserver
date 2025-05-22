/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:31:30 by sessarhi          #+#    #+#             */
/*   Updated: 2025/05/15 11:23:57 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include "ConfigData.hpp"
#include "Request.hpp"
class Response
{

public:
    Response(int status);
    Response(Request &, Server &);
    ~Response();


    int GetStatus()const;
    std::string GetData()const;
    void BuildResponse();
private:
    int StatusCode;
    std::map<std::string, std::string> headers;
    std::string data;
    std::string body;
};


#endif
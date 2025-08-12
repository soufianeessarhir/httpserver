/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:12 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/12 17:27:07 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             REQUEST_HPP

#define             REQUEST_HPP

#include            <string>

#include            <map>

#include            <sstream>

#include            <errno.h> 

#include            <cstdlib>

#include            <iostream>

#include            "HttpServer.hpp"

#include            "CGI.hpp" // added by eaboudi

#define CGiDir "/home/eaboudi/Desktop/httpserver/CGI-SCRIPTS" // added by eaboudi
// #define CGiDir "/Users/eaboudi/Desktop/httpserver/CGI-SCRIPTS" // added by eaboudi
class Request
{

public:

    Request():RequestStatusCode(200) ,IsComplete(false){}
    
    ~Request(){}
    
    bool            ParseHeaders(std::string );
    
    bool            ParseRequestLine(std::string& );
    
    bool            ExpectBody()const;
    
    size_t          GetContentLenght()const;
    
    
    bool            KeepAlive()const;
    
    static void     ToCanonical(std::string &);

    static void     trim(std::string &);

    bool            ParseUri();

    bool            Decode();

    static bool     OnlySpaces(std::string&);

    static bool     Haswhitespace(std::string&);

    void            NormalizePath();

    void            RemoveFromOutputpath(std::string &);



    



    
    std::string     GetUri();

    void            SetUri(std::string NewUri);

    const char*     GetBody()const;
    
    std::string     GetHeader(std::string )const;
    
    std::string     GetVersion()const;

    int             GetStatus()const;
    
    bool            GetIsComplet()const;

    std::string    GetMethod();

    bool            CheckField(const std::string &);
    

private:

    int                                 RequestStatusCode;

    bool                                IsComplete;
    
    std::string                         method;
    
    std::string                         uri;
    
    std::string                         version;
    
    std::map<std::string,std::string>   headers;

};


#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:12 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/21 13:35:06 by sessarhi         ###   ########.fr       */
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

    Request():RequestStatusCode(200){}
    
    ~Request(){}
    
    bool            ParseHeaders(std::string );
    
    bool            ParseRequestLine(std::string& );
    
    bool            ExpectBody()const;
    
    size_t          GetContentLenght()const;
    
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


    std::string    GetMethod();

    bool            CheckField(const std::string &);
    
    std::map<std::string,std::string>   headers;

private:

    int                                 RequestStatusCode;
    
    std::string                         method;
    
    std::string                         uri;
    
    std::string                         version;
    

};


#endif